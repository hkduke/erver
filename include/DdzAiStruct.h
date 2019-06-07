#pragma once
#include "CTType.h"
#include "../DdzAi/DdzAiLib/DdzAiInterface.h"


//AI返回的出牌消息
struct ai_action : public pai_interface_move
{
	unsigned int		uUserID;			//玩家ID
    unsigned int        uTime;              //时间
    unsigned char		event;				//事件ID
	unsigned char		callLandScore;		//如果是叫地主，返回叫地方分数

	ai_action()
	{
		reset();
	}

	~ai_action()
	{

	}

	void reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//传给AI消息
struct ai_event
{
	unsigned int		uUserID;
	unsigned int        uTime;
	unsigned char		event;
	unsigned char		landlord;
	unsigned char		outPaiIndex;
	unsigned char		pai_list[pai_i_type_max];
	unsigned char		pai_list2[pai_i_type_max];
	unsigned char		pai_list3[pai_i_type_max];
	unsigned char		grab_pai[3];
	pai_interface_move	limit_card;

	ai_event()
	{
		reset();
	}

	~ai_event()
	{

	}

	void reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//ai用户信息
struct ai_user
{
    unsigned int		uUserID;
    unsigned int        uEventTime;
};