/* ----------------------------------------------------------------------------------------------------------
 
	Popcap Shanghai studio

	Created by Zhang Hui in 2009-1-20
 ----------------------------------------------------------------------------------------------------------*/
#ifndef __Define_h__
#define __Define_h__

#if defined(_LINUX)	//Linux platform

typedef unsigned char			BYTE;
typedef unsigned short			WORD;
typedef unsigned short			USHORT;
typedef unsigned int			ULONG;
typedef unsigned int			UINT;
typedef int						INT32;

#else	// Other Platfrom (win32, etc...)
	// nothing to define.
#endif
#endif
