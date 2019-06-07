/** \file Thread.h
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
#ifndef _SOCKETS_Thread_H
#define _SOCKETS_Thread_H

#include "sockets-config.h"
#ifdef _WIN32
#else
#include <pthread.h>
#endif
#include "Semaphoreex.h"
#include "CTType.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _WIN32
// to be
//typedef DWORD  threadfunc_t;
//typedef LPVOID threadparam_t;
//#define STDPREFIX WINAPI
typedef unsigned threadfunc_t;
typedef CT_VOID * threadparam_t;
#define STDPREFIX __stdcall
#else

typedef CT_VOID * threadfunc_t;
typedef CT_VOID * threadparam_t;
#define STDPREFIX
#endif

/** \defgroup threading Threading */
/** Thread base class. 
The Thread class is used by the resolver (ResolvServer) and running a detached socket (SocketThread). 
When you know some processing will take a long time and will freeze up a socket, there is always the 
possibility to call Detach() on that socket before starting the processing.
When the OnDetached() callback is later called the processing can continue, now in its own thread.
	\ingroup threading */
class CThread
{
public:
	CThread(CT_BOOL release = CT_FALSE);
	virtual ~CThread();

	static threadfunc_t STDPREFIX StartThread(threadparam_t);

	//打开
	virtual CT_VOID Open(CT_VOID) = 0;
	//运行
	virtual CT_VOID Run(CT_VOID) = 0;

public:
#ifdef _WIN32
	enum CTP_PRIORITY {CTP_H=THREAD_PRIORITY_HIGHEST , CTP_N=0, CTP_L=-2};	
#else
	enum CTP_PRIORITY {CTP_H=2 , CTP_N=0, CTP_L=-2};
#endif
	//设置线程优先级
	CT_BOOL  SetPriority(CTP_PRIORITY cp);
	//创建线程
	CT_BOOL Create(CT_VOID* pPm);

#ifdef _WIN32
	HANDLE GetThread() { return m_thread; }
	CT_DWORD GetThreadId() { return m_dwThreadId; }
#else
	pthread_t GetThread() { return m_thread; }
#endif

	CT_BOOL IsRunning();
	CT_VOID SetRunning(CT_BOOL x);
	CT_BOOL IsReleased();
	CT_VOID SetRelease(CT_BOOL x);
	CT_BOOL DeleteOnExit();
	CT_VOID SetDeleteOnExit(CT_BOOL x = CT_TRUE);
	CT_BOOL IsDestructor();

	CT_VOID Start();

	CT_VOID Stop();

	CT_VOID Wait();

protected:
#ifdef _WIN32
	DT_HANDLE m_thread;
	CT_DWORD m_dwThreadId;
#else
	pthread_t m_thread;
	pthread_attr_t m_attr;
#endif

protected:
	CThread(const CThread& ) {}
	CThread& operator=(const CThread& ) { return *this; }
	CTSemaphore m_sem;
	CT_BOOL m_running;
	CT_BOOL m_release;
	CT_BOOL m_b_delete_on_exit;
	CT_BOOL m_b_destructor;
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_Thread_H

