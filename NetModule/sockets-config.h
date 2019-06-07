#ifndef		__DEF_SOCKETS_CONFIG_H__
#define		__DEF_SOCKETS_CONFIG_H__

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifndef _RUN_DP

#undef ENABLE_POOL
#undef ENABLE_SOCKS4
#undef ENABLE_IPV6

#endif	//_RUN_DP

//#if operator system is WIN32, then define  _OS_WIN32_CODE
//#if operator system is LINUX, then define  _OS_LINUX_CODE
//#if operator system is FREEBSD(MAC), then define _OS_FREEBSD_CODE
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define	_OS_WIN32_CODE
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define	_OS_LINUX_CODE
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define	_OS_MAC_CODE
#endif


/* Define NO_GETADDRINFO if your operating system does not support
   the "getaddrinfo" and "getnameinfo" function calls. */
#define NO_GETADDRINFO


/* Ipv6 support. */
//#define ENABLE_IPV6


/* Connection pool support. */
#define ENABLE_POOL


/* Socks4 client support. */
#define ENABLE_SOCKS4


/* Enabled exceptions. */
#define ENABLE_EXCEPTIONS


/*Enabled print. */
#define ENABLE_PRINT

#ifdef SOCKETS_NAMESPACE
}
#endif

#endif		//__DEF_SOCKETS_CONFIG_H__