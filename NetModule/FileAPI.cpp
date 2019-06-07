//////////////////////////////////////////////////////////////////////
//
// FileAPI.cpp
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////

#include "FileAPI.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#ifdef _WIN32
#include <io.h>			// for _open()
#include <fcntl.h>		// for _open()/_close()/_read()/_write()...
#include <string.h>		// for memcpy()
#else
#include <sys/types.h>	// for open()
#include <sys/stat.h>	// for open()
#include <unistd.h>		// for fcntl()
#include <fcntl.h>		// for fcntl()
#include <sys/ioctl.h>	// for ioctl()
#include <errno.h>		// for errno
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CT_INT32 open_ex ( const CT_CHAR * filename , CT_INT32 flags ) 
{

#ifndef _WIN32
	CT_INT32 fd = open(filename,flags);
#else
	CT_INT32 fd = _open(filename,flags);
#endif
	if ( fd < 0 ) {

#ifndef _WIN32
		switch ( errno ) {
		case EEXIST : 
		case ENOENT  : 
		case EISDIR : 
		case EACCES : 
		case ENAMETOOLONG : 
		case ENOTDIR : 
		case ENXIO   : 
		case ENODEV  : 
		case EROFS   : 
		case ETXTBSY : 
		case EFAULT  : 
		case ELOOP   : 
		case ENOSPC  : 
		case ENOMEM  : 
		case EMFILE  : 
		case ENFILE  : 
		default :
			{
				break;
			}
		}//end of switch
#else
	// ...
#endif
	}

	return fd;

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CT_INT32 open_ex ( const CT_CHAR * filename , CT_INT32 flags , CT_INT32 mode ) 
{

#ifndef _WIN32
	CT_INT32 fd = open(filename,flags,mode);
#else
	CT_INT32 fd = _open(filename,flags,mode);
#endif

	if ( fd < 0 ) {
#ifndef _WIN32
		switch ( errno ) {
		case EEXIST : 
		case EISDIR : 
		case EACCES : 
		case ENAMETOOLONG : 
		case ENOENT  : 
		case ENOTDIR : 
		case ENXIO   : 
		case ENODEV  : 
		case EROFS   : 
		case ETXTBSY : 
		case EFAULT  : 
		case ELOOP   : 
		case ENOSPC  : 
		case ENOMEM  : 
		case EMFILE  : 
		case ENFILE  : 
		default :
			{
				break;
			}
		}//end of switch
#else
	// ...
#endif
	}

	return fd;
}


//////////////////////////////////////////////////////////////////////
//
// DT_UINT32 read_ex ( DT_INT32 fd , DT_VOID * buf , DT_UINT32 len ) 
//
//
// exception version of read()
//
// Parameters 
//     fd  - file descriptor
//     buf - reading buffer
//     len - reading length
//
// Return
//     length of reading bytes
//
//
//////////////////////////////////////////////////////////////////////
CT_UINT32 read_ex ( CT_INT32 fd , CT_VOID * buf , CT_UINT32 len ) 
{

#ifndef _WIN32
	CT_INT32 result = read ( fd , buf , len );
#else
	CT_INT32 result = _read ( fd , buf , len );
#endif

	if ( result < 0 ) {

#ifndef _WIN32
		switch ( errno ) {
			case EINTR : 
			case EAGAIN : 
			case EBADF : 
			case EIO : 
			case EISDIR : 
			case EINVAL : 
			case EFAULT : 
			case ECONNRESET :
			default : 
				{
					break;
				}
		}
#else
	// ...
#endif
	} else if ( result == 0 ) {
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
//
// DT_UINT32 write_ex ( DT_INT32 fd , DT_VOID * buf , DT_UINT32 len ) 
//
//
// exception version of write()
//
// Parameters 
//     fd  - file descriptor
//     buf - writing buffer
//     len - writing length
//
// Return
//     length of reading bytes
//
//
//////////////////////////////////////////////////////////////////////
CT_UINT32 write_ex ( CT_INT32 fd , const CT_VOID * buf , CT_UINT32 len ) 
{

#ifndef _WIN32
	CT_INT32 result = write ( fd , buf , len );
#else
	CT_INT32 result = _write ( fd , buf , len );
#endif

	if ( result < 0 ) {
		
#ifndef _WIN32
		switch ( errno ) {
			case EAGAIN : 
			case EINTR : 
			case EBADF : 
			case EPIPE : 
			case EINVAL: 
			case EFAULT: 
			case ENOSPC : 
			case EIO : 
			case ECONNRESET :
			default : 
				{
					break;
				}
		}
#else
	//...
#endif
	}

	return result;
}


//////////////////////////////////////////////////////////////////////
//
// DT_VOID close_ex ( DT_INT32 fd ) 
//
//
// exception version of close()
//
// Parameters
//     fd - file descriptor
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
CT_VOID close_ex ( CT_INT32 fd ) 
{

	if ( close(fd) < 0 ) {
#ifndef _WIN32
		switch ( errno ) {
			case EBADF : 
			default :
				{
					break;
				}
		}
#else
#endif
	}
}

//////////////////////////////////////////////////////////////////////
//
// DT_INT32 fcntl_ex ( DT_INT32 fd , DT_INT32 cmd ) 
//
//
// Parameters
//     fd  - file descriptor
//     cmd - file control command
//
// Return
//     various according to cmd
//
//
//////////////////////////////////////////////////////////////////////
CT_INT32 fcntl_ex ( CT_INT32 fd , CT_INT32 cmd ) 
{

#ifndef _WIN32
	CT_INT32 result = fcntl ( fd , cmd );
	if ( result < 0 ) {
		switch ( errno ) {
			case EINTR : 
			case EBADF : 
			case EACCES : 
			case EAGAIN : 
			case EDEADLK : 
			case EMFILE  : 
			case ENOLCK : 
			default : 
				{
					break;
				}
		}
	}
	return result;
#else
	return 0 ;
#endif
}

//////////////////////////////////////////////////////////////////////
//
// DT_INT32 fcntl_ex ( DT_INT32 fd , DT_INT32 cmd , DT_LONG32 arg ) 
//
//
// Parameters
//     fd  - file descriptor
//     cmd - file control command
//     arg - command argument
//
// Return
//     various according to cmd
//
//
//////////////////////////////////////////////////////////////////////
CT_INT32 fcntl_ex ( CT_INT32 fd , CT_INT32 cmd , CT_LONG32 arg ) 
{

#ifndef _WIN32
	CT_INT32 result = fcntl ( fd , cmd , arg );
	if ( result < 0 ) {
		switch ( errno ) {
			case EINTR : 
			case EINVAL : 
			case EBADF : 
			case EACCES : 
			case EAGAIN : 
			case EDEADLK : 
			case EMFILE  : 
			case ENOLCK : 
			default : 
				{
					break;
				}
		}
	}
	return result;
#else
	return 0 ;
#endif
}


//////////////////////////////////////////////////////////////////////
//
// DT_BOOL getfilenonblocking_ex ( DT_INT32 fd ) 
//
//
// check if this file is nonblocking mode
//
// Parameters
//     fd - file descriptor
//
// Return
//     DT_TRUE if nonblocking, DT_FALSE if blocking
//
//
//////////////////////////////////////////////////////////////////////
CT_BOOL getfilenonblocking_ex ( CT_INT32 fd ) 
{

#ifndef _WIN32
	CT_INT32 flags = fcntl_ex( fd , F_GETFL , 0 );
	return flags | O_NONBLOCK;
#else
	return CT_FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////
//
// DT_VOID setfilenonblocking_ex ( DT_INT32 fd , DT_BOOL on ) 
//
//
// make this file blocking/nonblocking
//
// Parameters
//     fd - file descriptor
//     on - DT_TRUE if nonblocking, DT_FALSE if blocking
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
CT_VOID setfilenonblocking_ex ( CT_INT32 fd , CT_BOOL on ) 
{

#ifndef _WIN32
	CT_INT32 flags = fcntl_ex( fd , F_GETFL , 0 );

	if ( on )
		// make nonblocking fd
		flags |= O_NONBLOCK;
	else
		// make blocking fd
		flags &= ~O_NONBLOCK;

	fcntl_ex( fd , F_SETFL , flags );
#else
#endif
}

//////////////////////////////////////////////////////////////////////
//
// DT_VOID ioctl_ex ( DT_INT32 fd , DT_INT32 request , DT_VOID * argp )
//
//
// exception version of ioctl()
//
// Parameters
//     fd      - file descriptor
//     request - i/o control request
//     argp    - argument
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
CT_VOID ioctl_ex ( CT_INT32 fd , CT_INT32 request , CT_VOID * argp )
{

#ifndef _WIN32
	if ( ioctl(fd,request,argp) < 0 ) {
		switch ( errno ) {
		case EBADF : 
		case ENOTTY : 
		case EINVAL : 
		default :
			{
				break;
			}
		}
	}
#else
#endif
}



//////////////////////////////////////////////////////////////////////
//
// DT_VOID setfilenonblocking_ex2 ( DT_INT32 fd , DT_BOOL on )
//
//
// make this stream blocking/nonblocking using ioctl_ex()
//
// Parameters
//     fd - file descriptor
//     on - DT_TRUE if nonblocking, DT_FALSE else
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
CT_VOID setfilenonblocking_ex2 ( CT_INT32 fd , CT_BOOL on )
{

#ifndef _WIN32
	CT_ULONG arg = ( on == CT_TRUE ? 1 : 0 );
	ioctl_ex(fd,FIONBIO,&arg);
#else
#endif
}


//////////////////////////////////////////////////////////////////////
//
// DT_UINT32 available_ex ( DT_INT32 fd )
//
//
// how much bytes available in this stream? using ioctl_ex()
//
// Parameters
//     fd - file descriptor
//
// Return
//     #bytes available
//
//
//////////////////////////////////////////////////////////////////////
CT_UINT32 availablefile_ex ( CT_INT32 fd )
{

#ifndef _WIN32
	CT_UINT32 arg = 0;
	ioctl_ex(fd,FIONREAD,&arg);
	return arg;
#else
	return 0;
#endif
}


//////////////////////////////////////////////////////////////////////
//
// DT_INT32 dup_ex ( DT_INT32 fd )
//
//
//////////////////////////////////////////////////////////////////////
CT_INT32 dup_ex ( CT_INT32 fd )
{

#ifndef _WIN32
	CT_INT32 newfd = dup(fd);
#else
	CT_INT32 newfd = _dup(fd);
#endif

	if ( newfd < 0 ) {
#ifndef _WIN32
		switch ( errno ) {
		case EBADF : 
		case EMFILE : 
		default :
			{
				break;
			}
		}//end of switch
#else
#endif
	}

	return newfd;
}


//////////////////////////////////////////////////////////////////////
//
// DT_LONG32 lseek_ex ( DT_INT32 fd , DT_LONG32 offset , DT_INT32 whence )
//
//
//////////////////////////////////////////////////////////////////////
CT_LONG32 lseek_ex ( CT_INT32 fd , CT_LONG32 offset , CT_INT32 whence )
{

#ifndef _WIN32
	CT_LONG32 result = lseek(fd,offset,whence);
	if ( result < 0 ) {
		switch ( errno ) {
		case EBADF : 
		case ESPIPE : 
		case EINVAL : 
		default :
			{
				break;
			}
		}
	}
#else
	CT_LONG32 result = _lseek(fd,offset,whence);
	if ( result < 0 ) {
	}
#endif

	return result;

}

CT_LONG32 tell_ex( CT_INT32 fd )
{

#ifndef _WIN32
	CT_LONG32 result = 0;
#else
	CT_LONG32 result = _tell(fd);
	if ( result < 0 ) {
	}
#endif

	return result ;
}


#ifdef SOCKETS_NAMESPACE
}
#endif




