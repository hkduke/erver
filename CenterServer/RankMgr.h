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

typedef std::vector<tagScoreRank> VecScoreRank;
typedef std::vector<tagSignInRank> VecSignInRank;
typedef std::vector<tagShareRank> VecShareRank;
typedef std::map<CT_DWORD, tagShareRank*> MapShareRank;
typedef std::vector<tagTodayOnlineRank> VecTodayOnlineRank;
typedef std::vector<tagTodayTaskFinishRank> VecTodayTaskRank;
#define SCORE_RANK_COUNT 50

class CRankMgr : public acl::singleton<CRankMgr>
{
public:
	CRankMgr();
	~CRankMgr();

public:
	CT_VOID	SetRedisPtr(acl::redis* pRedis);
	//CT_VOID	ClearScoreRank();
	//CT_VOID InsertScoreRank(tagScoreRank* pScoreRank);

	CT_VOID	ClearYesterdayScoreRank();
	CT_VOID InsertYesterdayScoreRank(tagScoreRank* pScoreRank);

	//CT_VOID ClearShareRank();
	//CT_VOID InsertShareRank(tagShareRank* pShareRank);

	//CT_VOID ClearSignInRank();
	//CT_VOID InsertSignInRank(tagSignInRank* pSigninRank);

	CT_VOID ClearTodayOnlineRank();
	CT_VOID InsertTodayOnlineRank(tagTodayOnlineRank* pOnlineRank);

	CT_VOID ClearTodayTaskRank();
	CT_VOID InsertTodayTaskRank(tagTodayTaskFinishRank* pTaskRank);

	CT_VOID SendScoreRankToClient(CT_DWORD dwUserID);
	CT_VOID SendYesterScoreRank(CT_DWORD dwUserID);
	CT_VOID SendTodayOnlineRank(CT_DWORD dwUserID);
	CT_VOID SendTodayTaskRank(CT_DWORD dwUserID);

public:
	inline CT_LONGLONG	GetUserEarnScore(CT_DWORD dwUserID);
	inline CT_DWORD 	GetUeserTodayOnlineTime(CT_DWORD dwUserID);
	inline CT_VOID      GetUserTaskFinishInfo(CT_DWORD dwUserID, CT_WORD& wFinishCount, CT_DWORD& dwRewardScore);
	//inline tagShareRank* GetUserShareRank(CT_DWORD dwUserID);

private:
	//VecScoreRank		m_vecScoreRank;
	VecScoreRank		m_vecYesterdayScoreRank;
	//VecShareRank		m_vecShareRank;
	//MapShareRank		m_mapShareRank;
	//VecSignInRank		m_vecSignInRank;
	VecTodayOnlineRank	m_vecTodayOnlineRank;
	VecTodayTaskRank	m_vecTodayTaskRank;

	acl::redis*			m_pRedis;
};