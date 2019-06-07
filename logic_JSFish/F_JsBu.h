#ifndef _F_JSBU_H_
#define _F_JSBU_H_
enum message
{
	SUB_S_CHANGE_SCENE=100,				//�任����
	SUB_S_SEND_BIRD = 101,					//��������Ϣ
	SUB_S_SEND_BIRD_PAUSE_LINEAR = 102,       //��ͣ(102)
	SUB_S_SEND_BIRD_ROUND = 103,				//103
	SUB_S_SEND_BIRD_LINEAR = 104,             //������������(104)
	SUB_S_CATCH_BIRD = 105,
	SUB_S_SEND_BULLET = 106,
	SUB_S_FIRE_FAILED = 107,

	SUB_C_FIRE = 108,
	SUB_C_CATCH_FISH = 109,
	
	STATUSFREE = 110,
	SUB_S_ENTER_ANDROID_INFO = 111
};
#endif