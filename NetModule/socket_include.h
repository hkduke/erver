#ifndef		__DEF_SOCKETS_INCLUDE_H__
#define		__DEF_SOCKETS_INCLUDE_H__

#include "sockets-config.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(disable:4514)
#endif

#ifdef _OS_WIN32_CODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <winsock2.h> 
#include <mswsock.h> 
#include <ws2tcpip.h>
#define		CT_API				WINAPI
typedef		HANDLE				DT_HANDLE;
typedef		SOCKET				DT_SOCKET;
typedef		int					DT_SOCKETLEN;
#define		SOCKET_ERROR		(-1)
#define		gerrid				WSAGetLastError()
#define		DT_THREAD_FN		LPTHREAD_START_ROUTINE

//无效的句柄
#define INVALID_HANDLE	(-1)
//无效的ID值
#define INVALID_ID		(-1)

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif
#ifndef int64_t
typedef __int64 int64_t;
#endif
typedef DT_SOCKETLEN socklen_t;
#pragma comment(lib, "ws2_32.lib")

//#define strcasecmp _stricmp
#define snprintf sprintf_s
#define vsnprintf vsprintf_s

#elif defined(_OS_LINUX_CODE)
#include <cstdlib>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/epoll.h>

#define		CT_API			
typedef		int					DT_HANDLE;
typedef		int					DT_SOCKET;
typedef		socklen_t			DT_SOCKETLEN;
typedef		CT_VOID*			(*DT_THREAD_FN) (CT_VOID*);
#define		SOCKET_ERROR		(-1)
//#define		INVALID_SOCKET		(-1)
#define		gerrid				errno

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
typedef DT_SOCKETLEN socklen_t;

#elif defined(_OS_MAC_CODE)
#include <cstdlib>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/event.h>
#include <signal.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

#define		CT_API			
typedef		int					DT_HANDLE;
typedef		int					DT_SOCKET;
typedef		socklen_t			DT_SOCKETLEN;
typedef		CT_VOID*			(*DT_THREAD_FN) (CT_VOID*);
#define		SOCKET_ERROR		(-1)
//#define		INVALID_SOCKET		(-1)
#define		gerrid				errno

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
typedef DT_SOCKETLEN socklen_t;

#define SOCK_FBSD_KQ_ERROR   (0x01)
#define SOCK_FBSD_KQ_READ    (0x02)
#define SOCK_FBSD_KQ_WRITE   (0X04)

#endif



// getaddrinfo / getnameinfo replacements
#ifdef NO_GETADDRINFO
#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 1
#endif
#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 1
#endif
#endif

#endif		//__DEF_SOCKETS_INCLUDE_H__












