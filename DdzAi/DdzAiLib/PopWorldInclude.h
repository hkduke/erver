/* ----------------------------------------------------------------------------------------------------------
 

 
	Created by Zhang Hui in 2009-1-20
 ----------------------------------------------------------------------------------------------------------*/
#ifndef __Include_h__
#define __Include_h__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <cerrno>
#include <cassert>
#include <cctype>

//============================ SERVER VERSION ============================
#define SERVER_VERSION "2.0.10.8"
//========================================================================

#if defined(_LINUX)
#	include <unistd.h>
#	include <signal.h>
#	include <libgen.h>
#	include <sys/file.h>
#	include <sys/ipc.h>
#	include <sys/time.h>
#	include <sys/shm.h>
#	include <sys/epoll.h>
#	include <sys/ioctl.h>
#	include <arpa/inet.h>

#	define closesocket		close
#	define SOCKET			int
#	define INVALID_SOCKET	(SOCKET)(~0)
#	define SOCKET_ERROR		(-1)

inline long GetTickCount()
{
	struct timeval tv; 
	gettimeofday(&tv, NULL); 
	return (tv.tv_sec * 1000 + tv.tv_usec/1000);
}

#elif defined(_WIN32)	// wrapper for win32

#	include <winsock2.h>
#	include <fcntl.h>
#	include <io.h>

	typedef int	socklen_t;
#	define epoll_event		int
#	define pthread_mutex_lock(a)
#	define pthread_mutex_unlock(a)
#	define pthread_mutex_t	int
#	define pthread_mutex_init(a, b)
#	define strcasecmp		_stricmp
#	define snprintf			_snprintf
#	define unlink			_unlink
#	define access			_access
#	define umask			_umask
#	define flock(a, b)		0
#	define signal(a, b)
#	define pid_t			int
#	define fork()			0
#	define setsid()
#	define SIG_IGN			0
#	define SIGPIPE			0
#	define sigemptyset(a)
#	define setpgrp()
#	define socketpair(a, b, c, d)	1
#	define usleep(a)		Sleep(a/1000)
#	define sleep(a)			Sleep(a*1000)
#	define dirname(a)		""
#	define mkdir(a, b)
#	define atoll			_atoi64
struct sigaction
{
	sigaction() {}
	sigaction(int, void *, void *) {}
	int sa_handler;
	int sa_flags;
};

inline void gettimeofday(timeval * tm, void*)
{
	DWORD tick = GetTickCount();
	tm->tv_sec = (long)tick/1000;
	tm->tv_usec = (long)((tick % 1000) * 1000); 
};

inline int open(const char *, int, int)
{
	return 0;
}
#else	//unknown platform

#error unsupported platfrom.

#endif


#if defined(assert)
#undef assert
#define assert(a)	{ if (!(a)) { Log_Msg(Log_Debug, "assert failed:" #a "\n"); } }
#endif

#endif
