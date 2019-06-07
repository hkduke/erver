#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "CMD_Inner.h"
#include "CMD_Maintain.h"

struct tagSystemMsg
{
	CT_DWORD		dwID;					//消息ID
	CT_CHAR			szMessage[512];			//消息内容
	CT_DWORD		dwInterval;				//时间间隔
	CT_DWORD		dwLastBroadcastTime;	//上一次广播时间
	CT_DWORD		dwValidTime;			//截至时间

};

typedef std::map<CT_BYTE, std::vector<tagSystemMsg>> MapSystemMsg;

class CSystemMsgMgr : public acl::singleton<CSystemMsgMgr>
{
public:
	CSystemMsgMgr();
	~CSystemMsgMgr();

public:
	CT_VOID		BroadcastGameScoreMsg(MSG_G2CS_WinScore_Info* pWinScoreInfo);
	CT_VOID		InsertSystemMsg(MSG_M2CS_SendSysMsg* pSystemMsg);
	CT_VOID		InsertSystemMsg(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID		BroadcastSystemMsg();
	CT_VOID 	SendSystemMsg(CT_DWORD dwUserID, CT_CHAR* szSystemMsg);
	CT_VOID 	RemoveSystemMsg(const CT_VOID* pData, CT_DWORD wDataSize);
	
private:
	MapSystemMsg	m_mapSystemMsg;
}; 