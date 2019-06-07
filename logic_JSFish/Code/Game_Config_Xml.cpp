#include "stdafx.h"
#include "Game_Config_Xml.h"
#include "GameControl.h"
#include "SXml.h"
#include "glog_wrapper.h"
using namespace SDK;

bool Game_Config_Xml::load_success_ = false;
extern SystemPondControl pond_control;						//ȫ�ֽ��ؿ���
extern int platformCode;
Game_Config_Xml::Game_Config_Xml(void):
scene_swith_time_(300)
{
	for (int i=0; i<MAX_BIRD_TYPE; i++)
		bird_config_[i].capture_probability = 0;

	mulriple_count_ = 0;
	memset(cannon_mulriple_, 0, sizeof(cannon_mulriple_));
	memset(bomb_radius_, 0, sizeof(bomb_radius_));
	bullet_radius_ = 10;

	//�������
	//bomb_radius_[BOMB_SMALL] = 200 * 200;
	//bomb_radius_[BOMB_LARGE] = 250 * 250;
}

Game_Config_Xml::~Game_Config_Xml(void)
{
}

//�����޸�����,���������������
bool Game_Config_Xml::load_game_config()
{
	//��ȡĿ¼
	std::string szPath = "";
	char config_name[255] = "";
	char horn_config_name[255] = "";
    szPath = Utility::GetCurrentDirectory();
	std::snprintf(config_name, sizeof(config_name), "%s/script/js_config/jsby_config.xml", szPath.c_str());
	std::snprintf(horn_config_name, sizeof(horn_config_name), "%s\\script\\js_config\\horn_config.ini", szPath.c_str());

	//m_hornû��ʹ������
	/*char OutBuf[255];
	memset(OutBuf, 0, sizeof(OutBuf));
	GetPrivateProfileString("level", "Horn1", "��ϲ<c=1>[%s]</c>��<c=2>[���ٲ�����ͨ��]</c>������%s�����<c=3>[%d]</c>��ң����һ�������",
		OutBuf, 255 * sizeof(char), horn_config_name);
	CopyMemory(m_horn[0], OutBuf, 255 * sizeof(char));
	ZeroMemory(OutBuf, sizeof(OutBuf));
	GetPrivateProfileString("level", TEXT("Horn2"), _T("��ϲ<c=1>[%s]</c>��<c=2>[���ٲ�����ͨ��]</c>һ����꣬ն��󽱣������<c=3>[%d]</c>��ң����ǰ�����¶��"),
		OutBuf, 255 * sizeof(TCHAR), horn_config_name);
	CopyMemory(m_horn[1], OutBuf, 255 * sizeof(char));*/
	/*hornBirdNumber = GetPrivateProfileInt("name", TEXT("NameNumber"), 9, horn_config_name);
	ZeroMemory(OutBuf, sizeof(OutBuf));
	TCHAR * p = OutBuf;
	TCHAR keyName[10];
	for (int index = 0; index < hornBirdNumber; index++)
	{
		wsprintf(keyName, TEXT("Name_%d"), index);
		ZeroMemory(OutBuf, sizeof(OutBuf));
		GetPrivateProfileString("name", keyName, _T("������"), OutBuf, 255 * sizeof(TCHAR), horn_config_name);
		CopyMemory(m_birdName[index], OutBuf, sizeof(m_birdName[index]));
	}*/

	SXml doc;
	if(!doc.LoadFile(config_name)){
		return false;
	}
	tinyxml2::XMLElement *game_config = doc.GetElement("GameConfig");
	scene_swith_time_ = game_config->FloatAttribute("scene_switch_time");
	char* temp = NULL;
	tinyxml2::XMLElement *cannon_mulriple = game_config->FirstChildElement("CannonMulriple");
	const char* attri = cannon_mulriple->Attribute("mulriple");
	temp = NULL;
	if(attri)
	{
		mulriple_count_ = 0;
		cannon_mulriple_[mulriple_count_++] = strtol(attri, &temp, 10);
		while (strcmp(temp, ""))
		{
			cannon_mulriple_[mulriple_count_++] = strtol(temp + 1, &temp, 10);
			if (mulriple_count_ >= MAX_CANNON_TYPE)
				break;
		}
	}
	if (cannon_mulriple_[0] == 0)
	{
		//CTraceService::TraceString(L"����ڱ�Ϊ0", TraceLevel_Exception);
		return false;
	}
	tinyxml2::XMLNode *bird_config = game_config->FirstChildElement("BirdConfig");
	temp = NULL;
	for (bird_config = bird_config->FirstChild();bird_config!=0;bird_config=bird_config->NextSibling())
	{
		tinyxml2::XMLElement *data = bird_config->ToElement();
		int bird_kind = data->IntAttribute("kind");
		bird_config_[bird_kind].mulriple_min = data->IntAttribute("mulriple_min");
		const char* bird_multiple_arr = data->Attribute("mulriple_extend");
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
				bird_config_[bird_kind].mulriple_extend[i] = strtod(bird_multiple_arr, &temp);
			else
				bird_config_[bird_kind].mulriple_extend[i] = strtod(temp + 1, &temp);

		}
		bird_config_[bird_kind].mulriple_max = data->IntAttribute("mulriple_max");
		float width = strtoul(data->Attribute("size"), &temp, 10);
		float height = strtoul(temp+1, NULL, 10);
		bird_config_[bird_kind].bird_size.set_size(width,height);
		bird_config_[bird_kind].speed = data->FloatAttribute("speed");
		bird_config_[bird_kind].capture_probability = data->DoubleAttribute("capture_probability");
		bird_config_[bird_kind].distribute_interval_min = data->IntAttribute("distribute_interval_min");
		bird_config_[bird_kind].distribute_interval_max = data->IntAttribute("distribute_interval_max");
		bird_config_[bird_kind].distribute_count_min = data->IntAttribute("distribute_count_min");
		bird_config_[bird_kind].distribute_count_max =data->IntAttribute("distribute_count_max");
		bird_config_[bird_kind].path_type = data->IntAttribute("path_type");
		bird_config_[bird_kind].over_zero_prob =data->IntAttribute("over_zero_double_prob");
		bird_config_[bird_kind].under_zero_prob = data->IntAttribute("under_zero_double_prob");

		//LOG(ERROR) << "fish_kind:"<<bird_kind << " mulriple_min:" << bird_config_[bird_kind].mulriple_min << " mulriple_max: "<< bird_config_[bird_kind].mulriple_max;
	}
	const char* bird_multiple_control_arr = game_config->FirstChildElement("FishMultipleContorl")->Attribute("multiple");
	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
			fish_multiple_control[i] = strtod(bird_multiple_control_arr, &temp);
		else
			fish_multiple_control[i] = strtod(temp + 1, &temp);
	}
	//一箭多雕
	tinyxml2::XMLNode * bird_same_config = game_config->FirstChildElement("BirdSameConfig");
	temp = NULL;
	int bird_type = 0;
	for (bird_same_config = bird_same_config->FirstChild(); bird_same_config!=0; bird_same_config = bird_same_config->NextSibling())
	{
		tinyxml2::XMLElement *data = bird_same_config->ToElement();
		bird_same_config_[bird_type].bird_type = data->IntAttribute("kind");
		float width = strtoul(data->Attribute("size"), &temp, 10);
		float height = strtoul(temp + 1, NULL ,10);
		bird_same_config_[bird_type].bird_size.set_size(width, height);
		bird_same_config_[bird_type].bird_count = data->IntAttribute("bird_count");
		bird_same_config_[bird_type].bird_type_max = data->IntAttribute("bird_type_max");
		bird_same_config_[bird_type].mulriple = data->FloatAttribute("multiple");
		const char* pro_same_probability = data->Attribute("capture_probability");
		for (int i = 0; i < bird_same_config_[bird_type].bird_type_max; i++)
		{
			if (i == 0)
				bird_same_config_[bird_type].capture_probability[i] = strtod(pro_same_probability, &temp);
			else
				bird_same_config_[bird_type].capture_probability[i] = strtod(temp +1, &temp);
		}
		bird_same_config_[bird_type].speed = data->FloatAttribute("speed");
		bird_same_config_[bird_type].distribute_interval_min = data->IntAttribute("distribute_interval_min");
		bird_same_config_[bird_type].distribute_interval_max = data->IntAttribute("distribute_interval_max");
		bird_same_config_[bird_type].distribute_count_min = data->IntAttribute("distribute_count_min");
		bird_same_config_[bird_type].distribute_count_max =data->IntAttribute("distribute_count_max");
		bird_same_config_[bird_type].path_type = data->IntAttribute("path_type");
		bird_type++;
	}
	//特殊鱼
	tinyxml2::XMLElement * bird_special_config = game_config->FirstChildElement("BridSpecialConfig");
	bird_special_config_.distribute_interval_min = bird_special_config->IntAttribute("distribute_interval_min");
	bird_special_config_.distribute_interval_max = bird_special_config->IntAttribute("distribute_interval_max");
	bird_special_config_.path_type = bird_special_config->IntAttribute("path_type");

	//小鱼群
	tinyxml2::XMLElement * bird_group_config = game_config->FirstChildElement("BirdGroupConfig");
	bird_group_config_.distribute_interval_min = bird_group_config->IntAttribute("distribute_interval_min");
	bird_group_config_.distribute_interval_max = bird_group_config->IntAttribute("distribute_interval_max");
	bird_group_config_.distribute_count_min = bird_group_config->IntAttribute("distribute_count_min");
	bird_group_config_.distribute_count_max = bird_group_config->IntAttribute("distribute_count_max");
	bird_group_config_.path_type = bird_group_config->IntAttribute("path_type");

	//鼓出红鱼
	tinyxml2::XMLElement *bird_red_bloating = game_config->FirstChildElement("BirdRedBloatingConfig");
	temp = NULL;
	bird_red_bloating_config_.bird_type_max = bird_red_bloating->IntAttribute("bird_type_max");
	const char* pro_red_bloating = bird_red_bloating->Attribute("capture_probability");
	for (int i = 0; i < bird_red_bloating_config_.bird_type_max; i++)
	{
		if (i == 0)
			bird_red_bloating_config_.capture_probability[i] = strtod(pro_red_bloating, &temp);
		else
			bird_red_bloating_config_.capture_probability[i] = strtod(temp + 1, &temp);
	}
	bird_red_bloating_config_.red_probability = bird_red_bloating->IntAttribute("red_probability");
	bird_red_bloating_config_.distribute_interval_min = bird_red_bloating->IntAttribute("distribute_interval_min");
	bird_red_bloating_config_.distribute_interval_max = bird_red_bloating->IntAttribute("distribute_interval_max");
	bird_red_bloating_config_.ring_count_min = bird_red_bloating->IntAttribute("ring_count_min");
	bird_red_bloating_config_.ring_count_max = bird_red_bloating->IntAttribute("ring_count_max");




	//连串红鱼
	tinyxml2::XMLElement *bird_red_series = game_config->FirstChildElement("BirdRedSeriesConfig");
	temp = NULL;
	bird_red_series_config_.bird_type_max = bird_red_series->IntAttribute("bird_type_max");
	const char* pro_red_series = bird_red_series->Attribute("capture_probability");
	for (int i = 0; i < bird_red_series_config_.bird_type_max; i++)
	{
		if (i == 0)
			bird_red_series_config_.capture_probability[i] = strtod(pro_red_series, &temp);
		else
			bird_red_series_config_.capture_probability[i] = strtod(temp + 1, &temp);
	}
	bird_red_series_config_.speed = bird_red_series->FloatAttribute("speed");
	bird_red_series_config_.red_probability = bird_red_series->IntAttribute("red_probability");
	bird_red_series_config_.distribute_interval_min = bird_red_series->IntAttribute("distribute_interval_min");
	bird_red_series_config_.distribute_interval_max = bird_red_series->IntAttribute("distribute_interval_max");
	bird_red_series_config_.distribute_count_min = bird_red_series->IntAttribute("distribute_count_min");
	bird_red_series_config_.distribute_count_max = bird_red_series->IntAttribute("distribute_count_max");
	bird_red_series_config_.path_type = bird_red_series->IntAttribute("path_type");
	//子弹配置
	tinyxml2::XMLNode * bullet_config = game_config->FirstChildElement("BulletConfig");
	for (bullet_config = bullet_config->FirstChild(); bullet_config!=0; bullet_config = bullet_config->NextSibling())
	{
		tinyxml2::XMLElement * data = bullet_config->ToElement();
		int bullet_kind = data->IntAttribute("kind");
		bullet_config_[bullet_kind].speed = data->FloatAttribute("speed");
		bullet_config_[bullet_kind].fire_interval = data->FloatAttribute("fire_interval");
	}
    /*新版捕鱼概率控制*/
    //把消耗写入强发库几率(万分比)
	tinyxml2::XMLElement* forceLibJoinOdd = game_config->FirstChildElement("ForceLibJoinOdd");
	g_game_control.m_bForceBigFishOdd = forceLibJoinOdd->IntAttribute("BigFish");
	g_game_control.m_bForceSmallFishOdd = forceLibJoinOdd->IntAttribute("SmallFish");
    //强发库分值＜当前命中大鱼分值*炮弹倍数*0.5
	tinyxml2::XMLElement *forceLibMustOdd = game_config->FirstChildElement("ForceLibMustOdd");
	g_game_control.m_bForceMustOdd = forceLibMustOdd->FloatAttribute("Odd");
    //强发库存储金额≥当前鱼分值的3倍
	tinyxml2::XMLElement *forceLibMutil = game_config->FirstChildElement("ForceLibMutil");
	g_game_control.m_bForceMulti = forceLibMutil->IntAttribute("Mutil");
    //连续N炮未命中大鱼, 提升倍数N
	tinyxml2::XMLElement *forceLibMissHit = game_config->FirstChildElement("ForceLib100Hit");
	g_game_control.m_bForceMissCount = forceLibMissHit->IntAttribute("Count");
	g_game_control.m_bForceUpgradeMulti = forceLibMissHit->IntAttribute("Mutil");
    //净分控制
	tinyxml2::XMLElement *forceLibControlScore = game_config->FirstChildElement("ForceLibControlScore");
	g_game_control.m_maxWinratio = forceLibControlScore->IntAttribute("maxwinratio");
	g_game_control.m_minWinratio = forceLibControlScore->IntAttribute("minwinratio");
	g_game_control.m_losSratio = forceLibControlScore->IntAttribute("lossratio");
	g_game_control.m_roomMaxbet = forceLibControlScore->IntAttribute("roommaxbet");
	g_game_control.m_fishMulitiple = forceLibControlScore->IntAttribute("fishmulitiple");
	g_game_control.m_wininf = forceLibControlScore->IntAttribute("wininf");
	g_game_control.m_loseinf = forceLibControlScore->IntAttribute("loseinf");
	g_game_control.m_MaxForceThreshold = forceLibControlScore->IntAttribute("winscore");
	g_game_control.m_MinForceThreshold = forceLibControlScore->IntAttribute("losescore");
	g_game_control.m_MaxForceThresholdAndGold = forceLibControlScore->FloatAttribute("winScoreMax");
	g_game_control.m_MinForceThresholdAndGold = forceLibControlScore->FloatAttribute("loseScoreMin");
	g_game_control.m_SystemPondTaxodd = forceLibControlScore->IntAttribute("m_SystemPondTaxodd");
    //控鱼规则
	tinyxml2::XMLNode *fishContrlConfig = game_config->FirstChildElement("FishContrlConfig");
	for (fishContrlConfig = fishContrlConfig->FirstChild(); fishContrlConfig!=0; fishContrlConfig = fishContrlConfig->NextSibling())
	{
		tinyxml2::XMLElement *data = fishContrlConfig->ToElement();
		ContorlFishData tempXml;
		tempXml.KindID = data->IntAttribute("kind");
		tempXml.ForceChouShui = data->IntAttribute("qfcs");
		tempXml.ScoreContorl = data->IntAttribute("jfkz");
		tempXml.ForceHuiBu = data->IntAttribute("qfhb");
		if (tempXml.KindID < MAX_BIRD_TYPE)
		{
			g_game_control.m_arrayFishContorlRule[tempXml.KindID].KindID = tempXml.KindID;
			g_game_control.m_arrayFishContorlRule[tempXml.KindID].ForceChouShui = tempXml.ForceChouShui;
			g_game_control.m_arrayFishContorlRule[tempXml.KindID].ForceHuiBu = tempXml.ForceHuiBu;
			g_game_control.m_arrayFishContorlRule[tempXml.KindID].ScoreContorl = tempXml.ScoreContorl;
		}

	}
    //系统抽水
	tinyxml2::XMLElement * gameTaxodd = game_config->FirstChildElement("GameTaxodd");
    //系统抽水万分比
	g_game_control.m_Taxodd = gameTaxodd->IntAttribute("odd");

    //系统抽水开关
	tinyxml2::XMLElement * gameTaxoddSwitch = game_config->FirstChildElement("GameTaxoddSwitch");
    //系统抽水开关(0：默认关闭，1：开启) 开启纯抽水开关后，被系统抽水的分数将不影响玩家个人净分和库存
	g_game_control.m_TaxoddSwitch = gameTaxoddSwitch->IntAttribute("open");

    //全局奖池线
	tinyxml2::XMLElement * systemPondLine = game_config->FirstChildElement("SystemPondLine");

	pond_control.m_systemPondLineOne = systemPondLine->DoubleAttribute("one");
	pond_control.m_systemPondLineTwo = systemPondLine->DoubleAttribute("two");
	pond_control.m_systemPondLineThree = systemPondLine->DoubleAttribute("three");
	pond_control.m_systemPondLineFour = systemPondLine->DoubleAttribute("four");

    //全局奖池系数
	tinyxml2::XMLElement * systemPondCoefficient = game_config->FirstChildElement("SystemPondCoefficient");
	pond_control.m_systemPondCoefficientOneMin = systemPondCoefficient->DoubleAttribute("oneMin");
	pond_control.m_systemPondCoefficientOneMax = systemPondCoefficient->DoubleAttribute("oneMax");
	pond_control.m_systemPondCoefficientTwoMin = systemPondCoefficient->DoubleAttribute("twoMin");
	pond_control.m_systemPondCoefficientTwoMax = systemPondCoefficient->DoubleAttribute("twoMax");
	pond_control.m_systemPondCoefficientThreeMin = systemPondCoefficient->DoubleAttribute("threeMin");
	pond_control.m_systemPondCoefficientThreeMax = systemPondCoefficient->DoubleAttribute("threeMax");
	pond_control.m_systemPondCoefficientFourMin = systemPondCoefficient->DoubleAttribute("fourMin");
	pond_control.m_systemPondCoefficientFourMax = systemPondCoefficient->DoubleAttribute("fourMax");
	pond_control.m_systemPondCoefficientFiveMin = systemPondCoefficient->DoubleAttribute("fiveMin");
	pond_control.m_systemPondCoefficientFiveMax = systemPondCoefficient->DoubleAttribute("fiveMax");

    //个人净分抽水万分比
	tinyxml2::XMLElement * forceTaxodd = game_config->FirstChildElement("ForceTaxodd");
	g_game_control.m_foceTaxodd = forceTaxodd->IntAttribute("odd");
    //净分抽水写入开关
	tinyxml2::XMLElement * forceTaxoddWriteToPondSwitch = game_config->FirstChildElement("ForceTaxoddWriteToPondSwitch");
	if(forceTaxoddWriteToPondSwitch==NULL)
	{
		g_game_control.m_foceTaxoddWriteSwitch=0;
	}
	else
	{
		g_game_control.m_foceTaxoddWriteSwitch = forceTaxoddWriteToPondSwitch->IntAttribute("open");
	}

    //全局开炮计数  决定全局系数变化
	tinyxml2::XMLElement * cofficientKeepTimes = game_config->FirstChildElement("Coefficientkeeptimes");
    //全局开炮计数  决定全局系数变化
	g_game_control.m_cofficientKeepTimesMin = cofficientKeepTimes->IntAttribute("min");
	g_game_control.m_cofficientKeepTimesMax = cofficientKeepTimes->IntAttribute("max");

	if (g_game_control.m_cofficientKeepTimesMax <= g_game_control.m_cofficientKeepTimesMin)
	{
		return false;
	}

    ////鱼阵时间
	//pugi::xml_node timerControl = game_config.child("SceneTime");
	//for (pugi::xml_node node = timerControl.first_child(); node; node = node.next_sibling())
	//{
	//	int sceneKind = node.attribute("kind").as_int(0);
	//	g_game_control.scene_keep_time_[sceneKind] = node.attribute("time").as_int(0);
	//}
    //日志开关
	tinyxml2::XMLElement * logSwitch = game_config->FirstChildElement("LogSwitch");
	g_game_control.logSwitch = logSwitch->IntAttribute("open");
	g_game_control.goldSwitch = logSwitch->IntAttribute("goldSwitch");

	load_success_ = true;

    //补充刷新鱼
	//ZeroMemory(add_fish_refresh_config_, sizeof(add_fish_refresh_config_));
	memset(add_fish_refresh_config_, 0 , sizeof(add_fish_refresh_config_));
	tinyxml2::XMLNode *add_fish_refresh = game_config->FirstChildElement("AddFishRefreshConfig");
	for (add_fish_refresh = add_fish_refresh->FirstChild(); add_fish_refresh!=0; add_fish_refresh = add_fish_refresh->NextSibling())
	{
		tinyxml2::XMLElement *data = add_fish_refresh->ToElement();
		AddFishRefreshConfig tempXml;
		tempXml.refresh_probability = data->IntAttribute("prob");
		tempXml.refresh_interval = data->IntAttribute("interval");
		int kind = data->IntAttribute("kind");
		if (kind >= BIRD_TYPE_0 && kind < MAX_BIRD_TYPE)
		{
			add_fish_refresh_config_[kind] = tempXml;
		}
		else
		{
			return false;
		}
	}

    //强发库保底
	tinyxml2::XMLElement *_forceScoreProtect = game_config->FirstChildElement("ForceScoreProtect");
    //入场分数百分比
	g_game_control.forceScoreProtect.enterScorePercent = _forceScoreProtect->IntAttribute("enterScorePercent");
    //当前炮倍的倍数
	g_game_control.forceScoreProtect.nowBulletMultiple = _forceScoreProtect->IntAttribute("nowBulletMultiple");
    //两次触发之间的炮数间隔
	g_game_control.forceScoreProtect.touchInterval = _forceScoreProtect->IntAttribute("touchInterval");


    //入场延迟
	tinyxml2::XMLElement *_timesDelay = game_config->FirstChildElement("TimesDelay");
	g_game_control.timesDelay.min = _timesDelay->IntAttribute("min");
	g_game_control.timesDelay.max = _timesDelay->IntAttribute("max");

    //输赢强控机制
	tinyxml2::XMLNode *_winForceControl = game_config->FirstChildElement("WinForceControl");
	int configCount = 0;
	for (_winForceControl = _winForceControl->FirstChild(); _winForceControl!=0; _winForceControl = _winForceControl->NextSibling())
	{
		tinyxml2::XMLElement *data = _winForceControl->ToElement();

		WinForceControl tempXml;
        //触发权重
		tempXml.prob           = data->IntAttribute("prob");
        //叠加百分率
		tempXml.addodds        = data->IntAttribute("addodds");
        //控制分值下限
		tempXml.ControlptsMin  = data->DoubleAttribute("ControlptsMin");
        //控制分值上限
		tempXml.ControlptsMax  = data->DoubleAttribute("ControlptsMax");
        //控制分值入场百分比下限
		tempXml.ControlperMin  = data->IntAttribute("ControlperMin");
        //控制分值入场百分比上限
		tempXml.ControlperMax  = data->IntAttribute("ControlperMax");
        //回弹百分比下限
		tempXml.RecoverperMin  = data->IntAttribute("RecoverperMin");
        //回弹百分比上限
		tempXml.RecoverperMax  = data->IntAttribute("RecoverperMax");
        //回弹叠加百分率
		tempXml.Recoveraddodds = data->IntAttribute("Recoveraddodds");
        //结束延迟下限
		tempXml.NextDelayMin   = data->IntAttribute("NextDelayMin");
        //结束延迟上限
		tempXml.NextDelayMax   = data->IntAttribute("NextDelayMax");

		if (tempXml.ControlptsMin >= tempXml.ControlptsMax || 
			tempXml.ControlperMin >= tempXml.ControlperMax || 
			tempXml.RecoverperMin >= tempXml.RecoverperMax || 
			tempXml.NextDelayMin  >= tempXml.NextDelayMax)
		{
			//CTraceService::TraceString(L"输赢强控机制配置错误", TraceLevel_Debug);
			return false;
		}
		else
		{
			g_game_control.winForceControlArr[configCount++] = tempXml;
		}
		if (configCount > MAX_CONTROL_NUM)
		{
			//CTraceService::TraceString(L"输赢强控机制最大条数配置错误", TraceLevel_Debug);
			return false;
		}
	}

    //输赢强控奖池线
	tinyxml2::XMLNode *_winForcePondLine = game_config->FirstChildElement("WinForcePondLine");
	configCount = 0;
	g_game_control.winForcePondLine.lineCount = 0;
	for ( _winForcePondLine= _winForcePondLine->FirstChild(); _winForcePondLine!=0; _winForcePondLine = _winForcePondLine->NextSibling())
	{
		tinyxml2::XMLElement *data = _winForcePondLine->ToElement();
		g_game_control.winForcePondLine.pondLine[configCount++] = data->DoubleAttribute("value");
		if (configCount != 1 && g_game_control.winForcePondLine.pondLine[configCount] <= g_game_control.winForcePondLine.pondLine[configCount - 1])
		{
			//CTraceService::TraceString(L"输赢强控奖池线配置错误", TraceLevel_Debug);
			return false;
		}
		if (configCount > MAX_CONTROL_NUM - 1)
		{
			//CTraceService::TraceString(L"输赢强控奖池线条数配置错误", TraceLevel_Debug);
			return false;
		}
		g_game_control.winForcePondLine.lineCount = configCount;
	}

    //区间对应可选择组ID
	tinyxml2::XMLNode *_winForceSelectConfig = game_config->FirstChildElement("WinForceSelectConfig");
	configCount = 0;
	for (_winForceSelectConfig = _winForceSelectConfig->FirstChild(); _winForceSelectConfig!=0; _winForceSelectConfig = _winForceSelectConfig->NextSibling())	
	{
		tinyxml2::XMLElement *data = _winForceSelectConfig->ToElement();
		WinForceSelectConfig tempXml;
		const char* attri = data->Attribute("groupID");
		if (attri)
		{
			int type_count = 0;
			int groupID = strtol(attri, &temp, 10);
			if (0 > groupID || groupID >= MAX_CONTROL_NUM)
				continue;

			g_game_control.winForceSelectConfig[configCount].groupIndex[type_count] = groupID;
			type_count++;
			while (strcmp(temp, ""))
			{
				int groupID = strtol(temp + 1, &temp, 10);
				if (0 > groupID || groupID >= MAX_CONTROL_NUM)
					continue;
				if (type_count > MAX_CONTROL_NUM)
					continue;
				g_game_control.winForceSelectConfig[configCount].groupIndex[type_count++] = groupID;
			}
			g_game_control.winForceSelectConfig[configCount].groupCount = type_count;
		}
		configCount++;
		if (configCount > MAX_CONTROL_NUM)
		{

			//CTraceService::TraceString(L"�����Ӧ��ѡ����ID���ô���", TraceLevel_Debug);
			return false;
		}
	}

    //回弹控制
	tinyxml2::XMLElement *RecoverperControl = game_config->FirstChildElement("RecoverperControl");
    //玩家输赢回弹控制
	g_game_control.WinToLoseAddProb         = RecoverperControl->FloatAttribute("WinToLoseAddProb");
	g_game_control.LoseToWinAddProb         = RecoverperControl->FloatAttribute("LoseToWinAddProb");

	SCORE MaxinvLine = 0, AutoClear = 0, SafetyLine = 0;
    //奖池安全线
	SafetyLine = game_config->FirstChildElement("SafetyLine")->DoubleAttribute("line");
	SafetyLine *= 10;

    //最大库存线
	MaxinvLine = game_config->FirstChildElement("AutoPond")->DoubleAttribute("MaxinvLine");
	MaxinvLine *=10;

    //自动清除
	AutoClear = game_config->FirstChildElement("AutoPond")->DoubleAttribute("AutoClear");
	AutoClear *= 10;

	if (MaxinvLine < AutoClear)
	{
		//CTraceService::TraceStringEx(TraceLevel_Exception, L"MaxinvLine < AutoClear");
		return false;
	}

	/*
	//这几个值最好可以从后台传送过来，
    m_low_stockInfo.valueA = game_config->FirstChildElement("Kill_Award_Score_low_room")->IntAttribute("value_A");
    m_low_stockInfo.valueB = game_config->FirstChildElement("Kill_Award_Score_low_room")->IntAttribute("value_B");
    m_low_stockInfo.valueC = game_config->FirstChildElement("Kill_Award_Score_low_room")->IntAttribute("value_C");
    m_low_stockInfo.valueD = game_config->FirstChildElement("Kill_Award_Score_low_room")->IntAttribute("value_D");
    //第一次启动服务器从配置读取，关服写入数据库，以后启动服务器就从数据库中读取
    m_low_stockInfo.curStockValue = (m_low_stockInfo.valueB + m_low_stockInfo.valueC) / 2;
    m_low_stockInfo.nRevenueCount = game_config->FirstChildElement("Kill_Award_Score_low_room")->IntAttribute("nRevenueCount");

    m_middle_stockInfo.valueA = game_config->FirstChildElement("Kill_Award_Score_middle_room")->IntAttribute("value_A");
    m_middle_stockInfo.valueB = game_config->FirstChildElement("Kill_Award_Score_middle_room")->IntAttribute("value_B");
    m_middle_stockInfo.valueC = game_config->FirstChildElement("Kill_Award_Score_middle_room")->IntAttribute("value_C");
    m_middle_stockInfo.valueD = game_config->FirstChildElement("Kill_Award_Score_middle_room")->IntAttribute("value_D");
    m_middle_stockInfo.curStockValue = (m_middle_stockInfo.valueB + m_middle_stockInfo.valueC) / 2;
    m_middle_stockInfo.nRevenueCount = game_config->FirstChildElement("Kill_Award_Score_middle_room")->IntAttribute("nRevenueCount");

    m_high_stockInfo.valueA = game_config->FirstChildElement("Kill_Award_Score_high_room")->IntAttribute("value_A");
    m_high_stockInfo.valueB = game_config->FirstChildElement("Kill_Award_Score_high_room")->IntAttribute("value_B");
    m_high_stockInfo.valueC = game_config->FirstChildElement("Kill_Award_Score_high_room")->IntAttribute("value_C");
    m_high_stockInfo.valueD = game_config->FirstChildElement("Kill_Award_Score_high_room")->IntAttribute("value_D");
    m_high_stockInfo.curStockValue = (m_high_stockInfo.valueB + m_high_stockInfo.valueC) / 2;
    m_high_stockInfo.nRevenueCount = game_config->FirstChildElement("Kill_Award_Score_high_room")->IntAttribute("nRevenueCount");
    */


    //最大库存线
	g_game_control.MaxinvLine            = MaxinvLine;
    //自动清除
	g_game_control.AutoClear             = AutoClear;
    //奖池安全线
	g_game_control.SafetyLine            = SafetyLine;

    //开炮控制
    //开炮间隔
	g_game_control.fireInterval          = 200.0;
    //开炮叠加概率
	g_game_control.fireAddProb           = 0.8;

    //入场金币控制
    //入场金币系数
	g_game_control.enterScoreCoefficient = 5;
    //入场叠加概率
	g_game_control.enterAddProb          = 1.0;

	LOG(ERROR) << "Game_Config_Xml 加载完成";
	return true;
}
