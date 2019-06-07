/** \file Thread.cpp
 **	\date  2004-10-30
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2011  Anders Hedstrom

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
#include "socket_include.h"
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#include "Thread.h"
#include "Utility.h"
#include "exam_assert.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif

CThread::CThread(CT_BOOL release)
:m_thread(0)
,m_running(CT_FALSE)
,m_release(CT_FALSE)
,m_b_delete_on_exit(CT_FALSE)
,m_b_destructor(CT_FALSE)
{
#ifdef _WIN32
//	m_thread = ::CreateThread(NULL, 0, StartThread, this, 0, &m_dwThreadId);
	m_thread = (HANDLE)_beginthreadex(NULL, 0, &StartThread, this, 0, &m_dwThreadId);
#else
	pthread_attr_init(&m_attr);
	pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_DETACHED);
	if (pthread_create(&m_thread,&m_attr, StartThread,this) == -1)
	{
		perror("Thread: create failed");
		SetRunning(CT_FALSE);
	}
#endif
	m_release = release;
	if (release)
		m_sem.Post();
}


CThread::~CThread()
{
	m_b_destructor = CT_TRUE;
	Stop();
}

//设置线程优先级
CT_BOOL  CThread::SetPriority(CTP_PRIORITY cp)
{
#ifdef _WIN32
	return (CT_BOOL)SetThreadPriority(m_thread, cp);
#else
	return CT_TRUE;
#endif
}

//创建线程
CT_BOOL CThread::Create(CT_VOID* pPm)
{
#ifdef _WIN32
	m_thread = CreateThread(0, 0, (DT_THREAD_FN)(CThread::StartThread), pPm, 0, 0);
	_EXAM_ASSERT(NULL!=m_thread);
	if (NULL!=m_thread)
	{
		return CT_TRUE;
	}else
	{
		return CT_FALSE;
	}
#else
	return pthread_create(&m_thread, 0, (DT_THREAD_FN)(CThread::StartThread), pPm) != 0;
#endif

}


threadfunc_t STDPREFIX CThread::StartThread(threadparam_t zz)
{
	/*
		Sleep here to wait for derived thread class constructor to setup
		vtable... hurts just looking at it
	*/
	CThread *p = (CThread *)zz;
	_EXAM_ASSERT(NULL!=p);

	Utility::Sleep(10);
	p -> SetRunning(CT_TRUE); // if return
	p -> Open();
	p -> Run();
	
	p -> SetRunning(CT_FALSE); // if return
	if (p -> DeleteOnExit() && !p -> IsDestructor())
	{
		delete p;
	}
#ifdef _WIN32
	_endthreadex(0);
#endif
	return (threadfunc_t)NULL;
}


CT_VOID CThread::Start()
{
	SetRunning(CT_TRUE);
	SetRelease(CT_TRUE);
}

CT_VOID CThread::Stop()
{
	if (m_running)
	{
		SetRelease(CT_TRUE);
		SetRunning(CT_FALSE);
		/*
			Sleep one second to give thread class Run method enough time to
			release from run loop
		*/
		Utility::Sleep(2000);
	}
#ifdef _WIN32
	if (m_thread)
		::CloseHandle(m_thread);
#else
	void* ret = NULL;
	CT_INT32 re = pthread_join(m_thread, &ret);
	pthread_attr_destroy(&m_attr);
#endif
	m_thread = NULL;
}


CT_BOOL CThread::IsRunning()
{
 	return m_running;
}


CT_VOID CThread::SetRunning(CT_BOOL x)
{
 	m_running = x;
}


CT_BOOL CThread::IsReleased()
{
 	return m_release;
}


CT_VOID CThread::SetRelease(CT_BOOL x)
{
 	m_release = x;
 	if (x)
 		m_sem.Post();
}


CT_BOOL CThread::DeleteOnExit()
{
	return m_b_delete_on_exit;
}


CT_VOID CThread::SetDeleteOnExit(CT_BOOL x)
{
	m_b_delete_on_exit = x;
}


CT_BOOL CThread::IsDestructor()
{
	return m_b_destructor;
}


CT_VOID CThread::Wait()
{
	m_sem.Wait();
}


#ifdef SOCKETS_NAMESPACE
}
#endif


