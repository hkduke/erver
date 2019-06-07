/******************************************************************
 Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
 File name:		CMD_ShareMsg.h
 Version:		1.0
 Author:			osc
 Create Date:	2014-04-24
 History:
 
 ******************************************************************/

#ifndef CMD_SHARE_MSG_HEAD_FILE
#define CMD_SHARE_MSG_HEAD_FILE

#include "CTType.h"
#include "CGlobalData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//错误消息码
#define		SUB_ERROR_CODE_MSG				(101)				//断开连接消息错误码【MSG_Share_ErrorCode】



//错误码消息
struct MSG_Share_ErrorCode
{
	CT_DWORD	dwErrorCode;
};


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif //CMD_SHARE_MSG_HEAD_FILE



