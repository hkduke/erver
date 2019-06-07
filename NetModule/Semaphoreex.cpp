/**
 **	\file Semaphore.cpp
 **	\date  2007-04-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007-2011  Anders Hedstrom

This library is made available under the terms of the GNU GPL, with
the additional exemption that compiling, linking, and/or using OpenSSL 
is allowed.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "Semaphoreex.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


// ---------------------------------------------------------------
#ifdef _WIN32

CTSemaphore::CTSemaphore(value_t start_val)
{
	m_handle = ::CreateSemaphore((LPSECURITY_ATTRIBUTES)NULL, start_val, 1, (LPCTSTR)NULL);
}


CTSemaphore::~CTSemaphore(CT_VOID)
{
	::CloseHandle(m_handle);
}


CT_INT32 CTSemaphore::Post(CT_VOID)
{
	return (::ReleaseSemaphore(m_handle, 1, (LPLONG)NULL) != 0) ? 0 : -1;
}


CT_INT32 CTSemaphore::Wait(CT_VOID)
{
	return (WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
}


CT_INT32 CTSemaphore::TryWait(CT_VOID)
{
	return -1; // %! not implemented
}


CT_INT32 CTSemaphore::GetValue(CT_INT32& i)
{
	return 0; // %! not implemented
}

// ---------------------------------------------------------------
#else

CTSemaphore::CTSemaphore(value_t start_val)
{
	sem_init(&m_sem, 0, start_val);
}


CTSemaphore::~CTSemaphore(CT_VOID)
{
	sem_destroy(&m_sem);
}


CT_INT32 CTSemaphore::Post(CT_VOID)
{
	return sem_post(&m_sem);
}


CT_INT32 CTSemaphore::Wait(CT_VOID)
{
	return sem_wait(&m_sem);
}


CT_INT32 CTSemaphore::TryWait(CT_VOID)
{
	return sem_trywait(&m_sem);
}


CT_INT32 CTSemaphore::GetValue(CT_INT32& i)
{
	return sem_getvalue(&m_sem, &i);
}

#endif


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif





