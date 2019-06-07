#pragma once
#include "UserMgr.h"
#include <map>
#include "Utility.h"

typedef std::map<CT_WORD, tagTaskModel> MapTaskModel;
typedef std::map<CT_WORD, tagUserTask>  MapUserTask;


class CGlobalUser;
class CUserTask
{
public:
	CUserTask();
	~CUserTask();

	CT_VOID Init(CGlobalUser* pUser, acl::redis* pRedis);

public:
	//第一次接任务
	CT_VOID TheFirstGetTask();
	//跨天重置日常任务和登录任务
	CT_VOID AcrossDayRetTask();
	//跨周重置周任务
	CT_VOID	AcrossWeekRetTask();

	//检测登录任务
	CT_VOID OnUserLogin();
	//用户充值
	CT_VOID OnUserRecharge(CT_DWORD dwTotalRecharge);
	//用户打牌
	CT_VOID OnUserAddScore(CT_WORD wGameRoomKind, CT_LONGLONG llAddScore);
	//用户上排行榜
	CT_VOID	OnUserInRank();
	//用户玩游戏
	CT_VOID OnUserPlayGame(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);

	//插入一条新任务
	CT_VOID InsertNewTask(const tagTaskModel* pTaskInfo);
	//设置任务
	CT_VOID InsertTask(tagUserTask& userTask);
	//重置任务
	CT_VOID ResetTask(CT_WORD wTaskID);
	//零点重置任务
	CT_VOID ResetTaskInMidnight(CT_WORD wTaskID);

	//插入任务到DB
	inline CT_VOID InsertTaskTodb(tagUserTask& userTask);
	//更新任务
	inline CT_VOID	UpdateTask(tagUserTask& userTask);
	//更新任务到客户端
	inline CT_VOID UpdateTaskToClient(tagUserTask& userTask);
	//更新任务到DB
	inline CT_VOID UpdateTaskTodb(tagUserTask& userTask);
	//更新到redis
	inline CT_VOID UpdateTaskToCache(tagUserTask& userTask);
	//插入任务奖励记录到数据库
	inline CT_VOID InsertTaskRewardTodb(CT_DWORD dwUserID, CT_WORD wTaskID, CT_LONGLONG llReward);

	//领取任务奖励
	CT_VOID GetTaskReward(CT_WORD wTaskID);

	//发送任务信息到客户端
	CT_VOID SendAllTaskToClient();

private:
	CGlobalUser*	 m_pUser;
	acl::redis*		 m_pRedis;
	MapUserTask	     m_mapUserTask;
};

class CUserTaskModel : public acl::singleton<CUserTaskModel>
{
public:
	CUserTaskModel();
	~CUserTaskModel();
	
public:
	CT_VOID InsertTaskModel(tagTaskModel* pTaskInfo);
	tagTaskModel* GetTaskModel(CT_WORD wTaskID);
	const MapTaskModel& GetAllTaskModel();

	CT_VOID AcrossTheDay();
	CT_VOID AcrossTheWeek();

	//重置数据库日常任务
	CT_VOID ResetDailyTaskInDB();

private:
	MapTaskModel		m_mapTaskModel;
};