//////////////////////////////////////////////////////////////////////
//
// FileAPI.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __FILE_API_H__
#define __FILE_API_H__

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "CTType.h"

//////////////////////////////////////////////////////////////////////
//
// Exception based system-call(API) Collection
//
//////////////////////////////////////////////////////////////////////

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

//
// exception version of open ()
//
CT_INT32 open_ex (const CT_CHAR* filename, CT_INT32 flags) ;

CT_INT32 open_ex (const CT_CHAR* filename, CT_INT32 flags, CT_INT32 mode) ;

//
// exception version of close ()
//
CT_VOID close_ex (CT_INT32 fd) ;

//
// exception version of read ()
//
CT_UINT32 read_ex (CT_INT32 fd, CT_VOID* buf, CT_UINT32 len) ;

//
// exception version of write ()
//
CT_UINT32 write_ex (CT_INT32 fd, const CT_VOID* buf, CT_UINT32 len) ;

//
// exception version of fcntl ()
//
CT_INT32 fcntl_ex (CT_INT32 fd, CT_INT32 cmd) ;

//
// exception version of fcntl ()
//
CT_INT32 fcntl_ex (CT_INT32 fd, CT_INT32 cmd, CT_LONG32 arg) ;

//
// is this stream is nonblocking?
//
// using fcntl_ex()
//
CT_BOOL getfilenonblocking_ex (CT_INT32 fd) ;

//
// make this strema blocking/nonblocking
//
// using fcntl_ex()
//
CT_VOID setfilenonblocking_ex (CT_INT32 fd, CT_BOOL on) ;

//
// exception version of ioctl ()
//
CT_VOID ioctl_ex (CT_INT32 fd, CT_INT32 request, CT_VOID* argp);
	
//
// make this stream blocking/nonblocking
//
// using ioctl_ex()
//
CT_VOID setfilenonblocking_ex2 (CT_INT32 fd, CT_BOOL on);

//
// how much bytes available in this stream?
//
// using ioctl_ex()
//
CT_UINT32 availablefile_ex (CT_INT32 fd);

//
// exception version of dup()
//
CT_INT32 dup_ex (CT_INT32 fd);

//
// exception version of lseek()
//
CT_LONG32 lseek_ex(CT_INT32 fd, CT_LONG32 offset, CT_INT32 whence);


CT_LONG32 tell_ex( CT_INT32 fd ) ;


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif


