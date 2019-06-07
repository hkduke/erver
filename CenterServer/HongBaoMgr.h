#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include <memory>
#include "CTType.h"
#include "GlobalStruct.h"
#include "Group.h"
#include "CMD_Plaza.h"
#include "CMD_Dip.h"

typedef std::map<CT_DWORD, tagHongBaoInfo> MapHongBaoInfo;

class CHongBaoMgr : public acl::singleton<CHongBaoMgr>
{
public:
	CHongBaoMgr();
	~CHongBaoMgr();

public:
	CT_VOID InsertHongBaoInfo(tagHongBaoInfo* pHongBaoInfo, CT_BOOL bUpdateToClient);

	CT_VOID SendHongBao(MSG_CS_Send_HongBao* pSendHongBao);
	CT_VOID GuessHongBao(MSG_CS_Guess_HongBao* pTakeHongBao);
	CT_VOID	QueryMyselfHongBao(MSG_CS_Query_Myself_HongBao* pMyselfHongBao);
	CT_VOID OperateGuessOkHongBao(MSG_CS_Operate_GuessOk_HongBao* pOperateGuessOk);
	CT_VOID TakeTimeOutHongBao(MSG_CS_Take_TimeOut_HongBao* pTakeTimeOutHongBao);
	
	CT_VOID	SendHongBaoCountToClient(CT_DWORD dwUserID);
	CT_VOID SendAllHongBaoInfoToClient(CT_DWORD dwUserID);

private:
	CT_DWORD		m_dwHongBaoCount;
	MapHongBaoInfo	m_mapHongbaoInfo;
};