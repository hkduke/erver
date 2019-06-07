/**
 **	\file Semaphore.h
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
#ifndef _SOCKETS_Semaphore_H
#define _SOCKETS_Semaphore_H
#include "CTType.h"
#include "sockets-config.h"
#include "socket_include.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#ifdef MACOSX
#include <sys/semaphore.h>
#else
#include <semaphore.h>
#endif
#endif


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _WIN32
typedef CT_LONG32 value_t;
#else
typedef CT_UINT32 value_t;
#endif

/** pthread semaphore wrapper.
	\ingroup threading */
class CTSemaphore
{
public:
	CTSemaphore(value_t start_val = 0);
	~CTSemaphore();

	/** \return 0 if successful */
	CT_INT32 Post(CT_VOID);
	/** Wait for Post
	    \return 0 if successful */
	CT_INT32 Wait(CT_VOID);

	/** Not implemented for win32 */
	CT_INT32 TryWait(CT_VOID);

	/** Not implemented for win32 */
	CT_INT32 GetValue(CT_INT32&);

private:
	CTSemaphore(const CTSemaphore& ) {} // copy constructor
	CTSemaphore& operator=(const CTSemaphore& ) { return *this; } // assignment operator
#ifdef _WIN32
	DT_HANDLE m_handle;
#else
	sem_t m_sem;
#endif
};




#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif
#endif // _SOCKETS_Semaphore_H

