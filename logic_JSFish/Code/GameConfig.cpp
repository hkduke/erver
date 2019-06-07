#include "stdafx.h"
#include "Exception.h"
#include "GameTable.h"
#include "GameConfig.h"

///////////////////////////////////////////////////////////////////////////
Game_Config::Game_Config(Game_Table *table)
{
	table_ = table;
	initialise();
}

Game_Config::~Game_Config()
{
	cleanup();
}

void Game_Config::initialise()
{
	//不使用屏蔽M4
	/*bullet_radius_ = 10;

	net_radius_[0] = 80;
	net_radius_[1] = 80;
    net_radius_[2] = 80;
    net_radius_[3] = 80;
    net_radius_[4] = 80;
    net_radius_[5] = 80;
    net_radius_[6] = 80;
	net_radius_[7] = 80;
	net_radius_[8] = 80;
	net_radius_[9] = 80;
	net_radius_[10] = 80;
	net_radius_[11] = 80;
    net_radius_[12] = 80;
    net_radius_[13] = 80;
    net_radius_[14] = 80;
    net_radius_[15] = 80;
    net_radius_[16] = 80;
	net_radius_[17] = 80;
	net_radius_[18] = 80;
	net_radius_[19] = 80;
	net_radius_[20] = 80;
	net_radius_[21] = 80;
    net_radius_[22] = 80;
    net_radius_[23] = 80;
    net_radius_[24] = 80;
    net_radius_[25] = 80;
    net_radius_[26] = 80;
	net_radius_[27] = 80;
	net_radius_[28] = 80;
	net_radius_[29] = 80;
	net_radius_[30] = 80;
	net_radius_[31] = 80;
    net_radius_[32] = 80;
    net_radius_[33] = 80;
    net_radius_[34] = 80;
    net_radius_[35] = 80;
    net_radius_[36] = 80;
	net_radius_[37] = 80;
	net_radius_[38] = 80;
	net_radius_[39] = 80;

	bird_size_[0].set_size(41, 37);
	bird_size_[1].set_size(23, 44);
	bird_size_[2].set_size(34, 55);
	bird_size_[3].set_size(34, 72);
	bird_size_[4].set_size(52, 82);
	bird_size_[5].set_size(41, 86);
	bird_size_[6].set_size(51, 106);
	bird_size_[7].set_size(53, 120);
	bird_size_[8].set_size(123, 114);
	bird_size_[9].set_size(65, 148);
	bird_size_[10].set_size(145, 139);
	bird_size_[11].set_size(82, 240);	
	bird_size_[12].set_size(102, 255);	
	bird_size_[13].set_size(107, 290);
	bird_size_[14].set_size(174, 300);
	bird_size_[15].set_size(120, 280);
	bird_size_[16].set_size(200, 330);
	bird_size_[17].set_size(290, 300);
	bird_size_[18].set_size(120, 300);
	bird_size_[19].set_size(90, 90);
	bird_size_[20].set_size(120, 120);
	bird_size_[21].set_size(90, 90);
	bird_size_[22].set_size(90, 90);
	bird_size_[23].set_size(90, 90);

	bird_price_[0] = 2;
	bird_price_[1] = 2;
	bird_price_[2] = 3;
	bird_price_[3] = 4;
	bird_price_[4] = 5;
	bird_price_[5] = 6;
	bird_price_[6] = 7;
	bird_price_[7] = 8;
	bird_price_[8] = 9;
	bird_price_[9] = 10;
	bird_price_[10] = 12;
	bird_price_[11] = 15;	
	bird_price_[12] = 18;
	bird_price_[13] = 20;
	bird_price_[14] = 25;
	bird_price_[15] = 30;
	bird_price_[16] = 40;
	bird_price_[17] = 120;
	bird_price_[18] = 300;
	bird_price_[19] = 180; // 随机数
	bird_price_[20] = 170; // 随机数
	bird_price_[21] = 200;
	bird_price_[22] = 160;
	bird_price_[23] = 190;

	bullet_price_[0] = 25;
	bullet_price_[1] = 50;
	bullet_price_[2] = 75;
	bullet_price_[3] = 100;
	bullet_price_[4] = 125;
	bullet_price_[5] = 150;
	bullet_price_[6] = 175;
	bullet_price_[7] = 200;
	bullet_price_[8] = 225;
	bullet_price_[9] = 250;
	bullet_price_[10] = 275;
	bullet_price_[11] = 300;
	bullet_price_[12] = 325;
	bullet_price_[13] = 350;
	bullet_price_[14] = 375;
	bullet_price_[15] = 400;
	bullet_price_[16] = 425;
	bullet_price_[17] = 450;
	bullet_price_[18] = 475;
	bullet_price_[19] = 500;
    bullet_price_[20] = 525;
	bullet_price_[21] = 550;
	bullet_price_[22] = 575;
	bullet_price_[23] = 600;
	bullet_price_[24] = 625;
	bullet_price_[25] = 650;
	bullet_price_[26] = 675;
	bullet_price_[27] = 700;
	bullet_price_[28] = 725;
	bullet_price_[29] = 750;
    bullet_price_[30] = 775;
	bullet_price_[31] = 800;
	bullet_price_[32] = 825;
	bullet_price_[33] = 850;
	bullet_price_[34] = 875;
	bullet_price_[35] = 900;
	bullet_price_[36] = 925;
	bullet_price_[37] = 950;
	bullet_price_[38] = 975;
	bullet_price_[39] = 1000;

	bird_probability_[0] = 150;
	bird_probability_[1] = 142;
	bird_probability_[2] = 138;
	bird_probability_[3] = 135;
	bird_probability_[4] = 125;
	bird_probability_[5] = 120;
	bird_probability_[6] = 115;
	bird_probability_[7] = 110;
	bird_probability_[8] = 106;
	bird_probability_[9] = 100;
	bird_probability_[10] = 90;
	bird_probability_[11] = 80;	
	bird_probability_[12] = 70;	
	bird_probability_[13] = 60;	
	bird_probability_[14] = 50;	
	bird_probability_[15] = 46;
	bird_probability_[16] = 42;
	bird_probability_[17] = 38;
	bird_probability_[18] = 34;	
	bird_probability_[19] = 30;
	bird_probability_[20] = 24;
	bird_probability_[21] = 28;
	bird_probability_[22] = 24;
	bird_probability_[23] = 20;

	bullet_probability_[0] = 2;
	bullet_probability_[1] = 2;
	bullet_probability_[2] = 2;
	bullet_probability_[3] = 2;
	bullet_probability_[4] = 4;
	bullet_probability_[5] = 4;
	bullet_probability_[6] = 4;
	bullet_probability_[7] = 4;
	bullet_probability_[8] = 6;
	bullet_probability_[9] = 6;
	bullet_probability_[10] = 6;
	bullet_probability_[11] = 6;
	bullet_probability_[12] = 8;
	bullet_probability_[13] = 8;
	bullet_probability_[14] = 8;
	bullet_probability_[15] = 8;
	bullet_probability_[16] = 10;
	bullet_probability_[17] = 10;
	bullet_probability_[18] = 10;
	bullet_probability_[19] = 10;
	bullet_probability_[20] = 12;
	bullet_probability_[21] = 12;
	bullet_probability_[22] = 12;
	bullet_probability_[23] = 12;
	bullet_probability_[24] = 14;
	bullet_probability_[25] = 14;
	bullet_probability_[26] = 14;
	bullet_probability_[27] = 14;
	bullet_probability_[28] = 16;
	bullet_probability_[29] = 16;
	bullet_probability_[30] = 16;
	bullet_probability_[31] = 16;
	bullet_probability_[32] = 20;
	bullet_probability_[33] = 20;
	bullet_probability_[34] = 20;
	bullet_probability_[35] = 20;
	bullet_probability_[36] = 22;
	bullet_probability_[37] = 22;
	bullet_probability_[38] = 22;
	bullet_probability_[39] = 22;

	bomb_radius_[20] = 150 * 150;
	bomb_radius_[22] = 180 * 180;
	bomb_radius_[23] = 150 * 150;

	storage_start_ = 200000;

	max_win_score_ = 10000000;
	max_lost_score_ = 250000;
	min_win_score_ = 750000;                
	min_lost_score_  = 500000; 

    energy_cannon_probability_ = 20;

	table_chi_fen_threshold_ = 100000;
	chi_fen_zhou_qi_ = 600;                

    less_lost_probability_ = 0.6 ;
    less_win_probability_ = 1.35 ; 
    more_lost_probability_ = 0.3; 
    more_win_probability_ = 1.6	;

	da_wu_gui_ = 0;

	std::ostringstream ostr;
	char app_name[255] = "金蟾捕鱼";
	char config_name[255] = "JcbyServer\\DaWuGui.ini";
	da_wu_gui_ = GetPrivateProfileInt(app_name, "DaWuGui", 0, config_name);*/
}

void Game_Config::cleanup()
{
}

void Game_Config::load_script()
{
	//不使用屏蔽M4
	/*
	std::ostringstream ostr;

	char result[255];
	char app_name[255] = "金蟾捕鱼";

	char config_name[255] = "";

	//获取目录
	char szPath[MAX_PATH]="";
	GetCurrentDirectory(CountArray(szPath),szPath);
	_sntprintf(config_name,CountArray(config_name),"%s\\JcbyServer\\Config.ini",szPath);

	storage_start_ = GetPrivateProfileInt(app_name, "StorageStart", 0, config_name);

	da_wu_gui_ = GetPrivateProfileInt(app_name, "DaWuGui", 0, config_name);

	max_win_score_ = GetPrivateProfileInt(app_name, "MaxWinScore", 1000000, config_name);
	max_lost_score_  = GetPrivateProfileInt(app_name, "MaxLostScore", 250000, config_name);
	min_win_score_ = GetPrivateProfileInt(app_name, "MinWinScore", 750000, config_name);
	min_lost_score_  = GetPrivateProfileInt(app_name, "MinLostScore", 500000, config_name);

    energy_cannon_probability_ = GetPrivateProfileInt(app_name, "EnergyCannonProbability", 10, config_name);

	table_chi_fen_threshold_ = GetPrivateProfileInt(app_name, "TableChiFenThreshold", 10, config_name);
	chi_fen_zhou_qi_ = GetPrivateProfileInt(app_name, "ChiFenZhouQi", 10, config_name);


	GetPrivateProfileString(app_name, TEXT("LessLostProbability"),TEXT("1.0"), result, 255, config_name);
    _stscanf( result, TEXT("%f"), &less_lost_probability_ );
	GetPrivateProfileString(app_name, TEXT("LessWinProbability"),TEXT("1.0"), result, 255, config_name);
	_stscanf( result, TEXT("%f"), &less_win_probability_ );
	GetPrivateProfileString(app_name, TEXT("MoreLostProbability"),TEXT("1.0"), result, 255, config_name);
	_stscanf( result, TEXT("%f"), &more_lost_probability_ );
	GetPrivateProfileString(app_name, TEXT("MoreWinProbability"),TEXT("1.0"), result, 255, config_name);
	_stscanf( result, TEXT("%f"), &more_win_probability_ );

	for (int i = 0; i < MAX_BIRD_TYPE; i++)
	{
		_sntprintf(result, 255, TEXT("BirdProbability%d"), i);
		bird_probability_[i] = GetPrivateProfileInt(app_name, result, 0, config_name); 
	}*/
}

int Game_Config::get_bird_price(uint8_t bird_type, uint8_t energy_cannon) const
{
	if (energy_cannon == 1)
	{
		return 2 * bird_price_[bird_type];
	}
	else 
		return bird_price_[bird_type];
}

int Game_Config::get_probability(int cannon_type, int bird_type)
{
	assert(cannon_type < MAX_CANNON_TYPE);
	assert(bird_type < MAX_BIRD_TYPE);

	float probability = 0;

	if (da_wu_gui_ && bird_type == 9)
	{
		probability = 10000;
	}
	else
	{
		probability = bird_probability_[bird_type];
	}

	return std::ceil(probability);
}


///////////////////////////////////////////////////////////////////////////
