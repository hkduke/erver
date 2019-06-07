#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include <memory>
#include "CTType.h"
#include "CMD_Inner.h"
#include "GlobalStruct.h"

class CUserTask;
class CGlobalUser
{
public:
	CGlobalUser();
	~CGlobalUser();

public:
	CT_VOID				SetUserInfo(tagGlobalUserInfo* pUserInfo);
	tagGlobalUserInfo*  GetUserInfo();
	inline CT_DWORD		GetUserID()						{ return m_userInfo.dwUserID; }
	inline CT_DWORD		GetUserServerID()				{ return m_userInfo.dwGameServerID; }
	inline CT_DWORD 	GetUserProxyID()				{ return m_userInfo.dwProxyServerID; }
	inline CT_BOOL		IsOnline()						{ return m_userInfo.bOnline; }
	inline CT_VOID		SetOnline(CT_BOOL bOnline);
	inline CT_DWORD		GetOffLineTime()				{ return m_dwOffTime; }
	inline CT_VOID		SetRechargeTips(CT_LONGLONG llRecharge, CT_BYTE cbType);
	CT_VOID				ResetRechargeTips();
	inline CT_DWORD		GetChannelID()					{ return m_userInfo.dwChannelID; }
	inline CT_DWORD		GetClientChannelID()			{ return m_userInfo.dwClientChannelID; }
	inline CT_DWORD     GetGroupID()                    { return m_userInfo.dwGroupID; }
	inline CT_BYTE		GetShowExchange()				{ return m_userInfo.cbShowExchange; }
	inline CT_BYTE 		GetProviceId()					{ return m_userInfo.cbProvinceCode; }

	inline CT_DWORD 	GetLastHeartBeatTime()			{ return m_userInfo.dwLastHeartBeatTime; }
	inline CT_VOID 		SetLastHeartBeatTime(CT_DWORD dwTime) { m_userInfo.dwLastHeartBeatTime = dwTime; }

	inline CT_BYTE		GetPlatformId()					{ return m_userInfo.cbPlatformId; }
	inline CT_BYTE 		GetMacheineType()				{ return  m_userInfo.cbMachineType; }


	CT_VOID				SetUserScoreInfo(UserScoreData* pScoreInfo);
	UserScoreData*		GetUserScoreInfo();

	CT_VOID				SetUserTask(std::unique_ptr<CUserTask>& pUserTask);

	std::unique_ptr<CUserTask>& GetUserTask() { return m_pUserTask; }

public:
	CT_VOID				SendRechargeTips();

private:
	struct tagRechargeTip	//充值提醒
	{
		CT_BOOL			bNewTips;
		CT_BYTE			cbType;
		CT_LONGLONG     m_llLastRecharge;
	};

	tagGlobalUserInfo	m_userInfo;
	UserScoreData		m_scoreData;
	CT_DWORD			m_dwOffTime;
	tagRechargeTip		m_rechargeTips;		//最后一次充值提醒

	std::unique_ptr<CUserTask> m_pUserTask;
};

typedef std::map<CT_DWORD, std::shared_ptr<CGlobalUser>> MapGlobalUser;
class CUserMgr : public acl::singleton<CUserMgr>
{
public:
	CUserMgr();
	~CUserMgr();

public:
	CT_VOID		SetRedisPtr(acl::redis* pRedis);
	CT_VOID		InsertUser(MSG_P2CS_Register_User* pRegisterUser, CT_BYTE cbScoreMode);
	CT_VOID		UpdateUserGsInfo(CT_DWORD dwUserID, CT_DWORD dwServerID);
	CT_VOID		DeleteUser(CT_DWORD dwUserID);
	CT_VOID		SetUserOnline(CT_DWORD dwUserID, CT_BOOL bOnline);

	CT_BOOL		IsUserOnline(CT_DWORD dwUserID);
	acl::aio_socket_stream*	FindUserProxySocksPtr(CT_DWORD dwUserID);
	tagGlobalUserInfo* GetUserInfo(CT_DWORD dwUserID);
	MapGlobalUser&	   GetAllGlobalUser() { return m_mapGlobalUser; }
	CGlobalUser*	   GetGlobalUser(CT_DWORD dwUserID);
	CUserTask*		   GetUserTaskPtr(CT_DWORD dwUserID);
	CT_DWORD		   GetUserChannelID(CT_DWORD dwUserID);
	CT_DWORD		   GetUserClientChannelID(CT_DWORD dwUserID);
  	CT_VOID 		   SetUserHeartBeatTime(CT_DWORD dwUserID, CT_DWORD dwTime);
  	CT_DWORD 		   GetUserHeartBeatTime(CT_DWORD dwUserID);

	CT_LONGLONG GetUserScore(CT_DWORD dwUserID);
	CT_LONGLONG GetUserBankScore(CT_DWORD dwUserID);
	CT_DWORD 	GetUserRecharge(CT_DWORD dwUserID);
	CT_VOID     SetUserRecharge(CT_DWORD dwUserID, CT_DWORD dwRecharge);
	CT_BYTE		GetUserShowExchange(CT_DWORD dwUserID);
	CT_VOID		AddUserGem(CT_DWORD dwUserID, CT_INT32 nAddGem);
	CT_VOID		AddUserScore(CT_DWORD dwUserID, CT_LONGLONG llAddScore);
	CT_VOID		AddUserGemEx(CT_DWORD dwUserID, CT_INT32 nAddGem, CT_BOOL bUpdateClient = false);
	CT_VOID		AddUserScoreEx(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_BOOL bUpdateClient = false);
	CT_VOID		AddUserBankScoreEx(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore, CT_BOOL bUpdateClient = false);
    CT_BYTE		GetUserPlatformId(CT_DWORD dwUserID);
	CT_VOID		InsertUserScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBankScore, CT_LONGLONG llSourceScore, \
											CT_LONGLONG llAddBankScore, CT_LONGLONG llAddScore, enScoreChangeType eType);

	//redis
	CT_VOID		UpdateUserVipLevel(CT_DWORD dwUserID,  CT_LONGLONG llAddScore, CT_DWORD dwTotalRecharge, CT_BYTE cbVipLevel, CT_BYTE cbVipLevel2);
	//加载金币场信息
	CT_VOID		LoadUserScoreInfo(CT_DWORD dwUserID, UserScoreData& scoreInfo);
	//更新金币场信息
	CT_VOID		UpdateUserScoreInfo(CT_DWORD dwUserID, MSG_Update_ScoreInfo* pAddScoreInfo);
	//加载任务信息
	CT_VOID		LoadUserTaskInfo(CT_DWORD dwUserID, std::shared_ptr<CGlobalUser>& pGlobalUser);
	//设置玩家可以显示兑换按钮
	CT_VOID		SetUserShowExchange(CT_DWORD dwUserID, CT_BYTE cbShow);

	//定时删除离线玩家
	CT_VOID		RemoveOffLineUser();

	//dip功能函数
	//查询在线玩家
	CT_VOID		QueryOnlineUser(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pDipSocket);
	CT_VOID		NotifyNewMail(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID		SendNewMailTip(CT_DWORD dwUserID);
	CT_VOID 	SendUserMail(tagUserMail* pUserMail);
	CT_VOID		UserRecharge(CT_DWORD dwUserID, CT_LONGLONG llRecharge, CT_BYTE cbType);

private:
	MapGlobalUser		m_mapGlobalUser;
	acl::redis*			m_pRedis;
}; 