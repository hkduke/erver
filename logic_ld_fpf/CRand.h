#ifndef CCRAND_H_
#define CCRAND_H_

#include "CTType.h"

//随机数
class CCRand
{
public:
	CCRand(void);
	~CCRand(void);
public:
	//随机数
	CT_INT32 rand(void);
	//设置种子
	CT_VOID srand(CT_UINT32 seed);
private:
	CT_INT32 rand_r(CT_UINT32 *seed);
private:
	CT_UINT32 m_iNext;
};

#endif






