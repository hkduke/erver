#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_jznc.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include <random>
#include <algorithm> // std::move_backward
#include <random> // std::default_random_engine
#include <chrono> // std::chrono::system_clock
#include "Utility.h"
#include "ITableFrame.h"


//宏定义
//////////////////////////////////////////////////////////////////////////
//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//游戏逻辑类
class CGameLogic
{
	//函数定义
public:
	//构造函数
	CGameLogic();
	//析构函数
	virtual ~CGameLogic();
    void SetGameRoomKindInfo(tagGameRoomKind* pGameRoomKindInfo){m_pGameRoomKindInfo = pGameRoomKindInfo;}
    void SetCtrlEffect(std::map<int , std::vector<CT_WORD >> & ctrlEffect);
	CT_WORD RandDice(int key);
    CT_VOID FillDice(int key);
private:
	std::map<int , std::vector<CT_WORD >>	m_mCtrlEffect;			//控制相对骰盅的骰面分布个数, key ---控制效果，value 每个面骰的个数，vector：0--1面个数，1--2面个数
    tagGameRoomKind*				            m_pGameRoomKindInfo;
private:
	std::map<int , std::vector<CT_WORD >>	m_mEffectDice;			//不用效果对应的骰子的值，key ---控制效果，value 每个面骰的值，vector：大小60，
private:
    std::random_device                          m_rd;
    std::mt19937                                m_gen;
    std::uniform_int_distribution<unsigned int> m_iProba;
};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


