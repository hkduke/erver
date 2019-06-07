#pragma once
//#if operator system is WIN32, then define  _OS_WIN32_CODE
//#if operator system is LINUX, then define  _OS_LINUX_CODE
//#if operator system is FREEBSD(MAC), then define _OS_MAC_CODE
#ifdef  _WIN32
#define		_OS_WIN32_CODE
#elif   __linux__
#define		_OS_LINUX_CODE
#elif   __APPLE__
#define     _OS_MAC_CODE
#endif

typedef          unsigned char			CT_UCHAR;
typedef          CT_UCHAR				CT_BYTE;
typedef          char					CT_CHAR;
typedef          int					CT_INT32;
typedef          unsigned int			CT_UINT32;
typedef          CT_UINT32				CT_DWORD;
typedef          unsigned short			CT_WORD;
typedef          short					CT_SHORT;
typedef          bool					CT_BOOL;
typedef          float					CT_FLOAT;
typedef          double					CT_DOUBLE;
typedef          long					CT_LONG;
typedef          CT_DWORD				CT_LONG32;
typedef          long long				CT_LONG64;
typedef			unsigned long			CT_ULONG;
typedef          CT_LONG64				CT_LONGLONG;
typedef			 unsigned long long		CT_ULONGLONG;
typedef          void					CT_VOID;
typedef          void(*CT_TIMER_FN) (CT_WORD id);

#define          CT_TRUE				true
#define          CT_FALSE				false

#ifdef _WIN32
typedef			__int64					CT_INT64;
typedef			unsigned __int64		CT_UINT64;
#else
#include <sys/types.h>
#define			CT_VOID					void
typedef			int64_t					CT_INT64;
typedef			u_int64_t				CT_UINT64;
#endif

//安全释放堆空间
#define		SAFE_DELETE_PTR(ptr)		{ if (NULL!=(ptr)) {delete (ptr); (ptr)=NULL;} }
#define     SAFE_DELETE_OBJECT(p)      do { delete (p); (p) = NULL; } while(0)
//#define     SAFE_DELETE_ARRAY(p)       do { if(p) { delete[] (p); (p) = NULL; } } while(0)
	
//根据指针值删除内存
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if( (x)!=NULL ) { delete (x); (x)=NULL; }
#endif
//根据指针值删除数组类型内存
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)	if( (x)!=NULL ) { delete[] (x); (x)=NULL; }
#endif
//根据指针调用free接口
#ifndef SAFE_FREE
#define SAFE_FREE(x)	if( (x)!=NULL ) { free(x); (x)=NULL; }
#endif
//根据指针调用Release接口
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if( (x)!=NULL ) { (x)->Release(); (x)=NULL; }
#endif

#ifndef _BIT32
#define _BIT32(x) ((x < 32) ? (1 << (x)) : 0) 					///< Returns value with bit x set (2^x)
#endif//_BIT32

#ifndef _SET_BIT32
#define _SET_BIT32(mark,bit,s) (s ? (mark |= _BIT32(bit)) : (mark &=~_BIT32(bit)))
#endif//_SET_BIT32

#ifndef _CHECK_BIT
#define _CHECK_BIT(mark,bit)	((mark & bit) > 0)				///检测是否存在相同位
#endif//_CHECK_BIT

#ifndef _BIT64
#define _BIT64(x) uint64((x < 64) ? ((uint64)1 << (x)) : 0) 	///< Returns value with 64 bit x set (2^x)
#endif//_BIT64

#ifndef _SET_BIT64
#define _SET_BIT64(mark,bit,s) (s ? (mark |= _BIT64(bit)) : (mark &=~_BIT64(bit)))
#endif//_SET_BIT64

#ifdef _OS_WIN32_CODE
#define		_snprintf_info				_snprintf_s
#define		_sscanf_info				sscanf_s
#define		_64bitFormat				%I64u	
#else
#define		_snprintf_info				snprintf
#define		_sscanf_info				sscanf
#define		_64bitFormat				%llu	
#endif


#ifndef _OPEN_TRY_CHECK_
#define	_OPEN_TRY_CHECK_
#endif

#ifdef _OPEN_TRY_CHECK_

#define		__TRY_START__				try {

#define		__TRY_END__                 }

#define		__CATCH_CHECK_START_		catch(...) {

#define		__CATCH_CHECK_END_          }

#else

#define		__TRY_START__	

#define		__TRY_END__		

#define		__CATCH_CHECK_START_		

#define		__CATCH_CHECK_END_  

#endif