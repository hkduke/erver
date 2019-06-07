#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>

//游戏逻辑类
class CGameLogic
{
	//变量定义
private:
	static CT_BYTE						m_cbCardListData[52];			//扑克定义

																		//函数定义
public:
	//构造函数
	CGameLogic();
	//析构函数
	virtual ~CGameLogic();
};


#endif		 


