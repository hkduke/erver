#include "stdafx.h"
#include "lockArray.h"
//#include <windows.h>



lockArray::lockArray(void *m_Lock)
{
	/*this->m_Lock = m_Lock;
	CRITICAL_SECTION *self_lock = (CRITICAL_SECTION *)this->m_Lock;
	EnterCriticalSection(self_lock);*/
}

lockArray::~lockArray()
{
	/*CRITICAL_SECTION *self_lock = (CRITICAL_SECTION *)this->m_Lock;
	LeaveCriticalSection(self_lock);*/
}
