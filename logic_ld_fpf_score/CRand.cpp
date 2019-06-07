#include "CRand.h"

const CT_DWORD	RAND_MAX_EX = 0x7FFF;

CCRand::CCRand(void):m_iNext(1)
{
}

CCRand::~CCRand(void)
{
}

CT_INT32 CCRand::rand_r(CT_UINT32 *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return (*seed % ((CT_UINT32)RAND_MAX_EX + 1));
}

CT_INT32 CCRand::rand(void)
{
	CT_INT32 iRand=rand_r(&m_iNext);
	return iRand;
}

CT_VOID CCRand::srand(CT_UINT32 seed)
{
	m_iNext = seed;
}
