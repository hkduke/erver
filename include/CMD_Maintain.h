#ifndef CMD_Maintain_H_
#define CMD_Maintain_H_
#include "CTType.h"
#include "CGlobalData.h"
#include "IGameOtherData.h"
#include "CTData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//工具和中心服务服通讯的消息定义
#define MSG_MCS_MAIN								60

//查询所有的服务器信息
#define SUB_M2CS_QUERY_ALL_SERVER					6000
#define SUB_M2CS_SET_SERVER_STATE					6001
#define SUB_M2CS_DISMISS_SERVER_PROOM				6002
#define SUB_M2CS_DISMISS_SERVER_PROOM_BY_ROOMNUM	6003
#define SUB_M2CS_REMOVE_USER_OUT_CACHE				6004
#define SUB_M2CS_SEND_SYS_MESSAGE					6005
#define SUB_M2CS_SET_RECHARGE_JUDGE					6006

#define SUB_CS2M_QUERY_ALL_SERVER					6500
#define SUB_CS2M_QUERY_SERVER_FINISH				6501
#define SUB_CS2M_SET_SERVER_STATE_SUCC				6502
#define SUB_CS2M_DISMISS_SERVER_RPOOM_SUCC			6503
#define SUB_CS2M_DISMISS_SERVER_RPOOM_BYNUM_SUCC	6504
#define SUB_CS2M_REMOVE_USER_OUT_CACHE_RESULT		6505
#define SUB_CS2M_SEND_SYS_MESSAGE					6506

struct MSG_M2CS_QueryServer
{
	CT_DWORD		dwServerID;						
};

struct MSG_M2CS_SetServerState
{		
	CT_DWORD		dwServerID;
	CT_BYTE			cbState;
};

struct MSG_M2CS_DismissPRoom
{
	CT_DWORD		dwServerID;
};

struct MSG_M2CS_DismissPRoom_ByRoomNum
{
	CT_DWORD		dwRoomNum;
};

struct MSG_M2CS_RemoveUserOutCache
{
	CT_DWORD		dwUserID;
};

struct MSG_M2CS_SendSysMsg
{
	CT_CHAR			szMessage[512];		//消息内容
	CT_DWORD		dwInterval;			//时间间隔
	CT_DWORD		dwValidTime;		//截至时间
	CT_BYTE 		cbPlatformId;		//平台ID
};

struct MSG_CS2M_CommonResult
{
	CT_BYTE			cbResult;
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif