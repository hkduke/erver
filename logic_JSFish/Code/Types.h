#ifndef __TYPES_H__
#define __TYPES_H__

///////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif

#define INVALID_ID -1
#define INVALID_CHAIR_ID 0xFFFF
#define INVALID_CHECK 0xFFFFFFFF
#define INVALID_BULLET_CREATE_TIME 0
///////////////////////////////////////////////////////////////////////////////////////

#endif