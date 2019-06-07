#include "stdafx.h"
#include "UserTask.h"
#include "GlobalEnum.h"
#include "NetModule.h"
#include "timeFunction.h"
#include "CMD_Plaza.h"
#include "GroupMgr.h"

extern CNetConnector *pNetDB;
CUserTask::CUserTask()
	:m_pUser(NULL)
	,m_pRedis(NULL)
{

}

CUserTask::~CUserTask()
{
	//LOG(WARNING) << "remove user task!";
}

CT_VOID CUserTask::Init(CGlobalUser* pUser, acl::redis* pRedis)
{
	m_pUser = pUser;
	m_pRedis = pRedis;
}

CT_VOID CUserTask::TheFirstGetTask()
{
	const MapTaskModel& mapTaskModel = CUserTaskModel::get_instance().GetAllTaskModel();
	for (auto& it : mapTaskModel)
	{
		tagUserTask userTask(it.first);

		m_mapUserTask.insert(std::make_pair(it.first, userTask));
	}
}

CT_VOID CUserTask::AcrossDayRetTask()
{
	for (auto& it : m_mapUserTask)
	{
		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		switch (pTaskMode->wTaskType)
		{
		case TASK_TYPE_DAILY:
		case TASK_TYPE_LOGIN:
		{
			it.second.Reset();
		}
		break;
		default:
			break;
		}
	}
}

CT_VOID	CUserTask::AcrossWeekRetTask()
{
	for (auto& it : m_mapUserTask)
	{
		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		switch (pTaskMode->wTaskType)
		{
		case TASK_TYPE_WEEKLY:
		{
			it.second.Reset();
		}
		break;
		default:
			break;
		}
	}
}

CT_VOID CUserTask::OnUserLogin()
{
	/*static tm timeNow;
	memset(&timeNow, 0, sizeof(timeNow));
	getLocalTime(&timeNow, time(NULL));

	bool bUpdate = false;
	for (auto& it : m_mapUserTask)
	{
		if (it.second.bFinish)
			continue;

		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		if (pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
		{
			continue;
		}

		switch (pTaskMode->wCompleteType)
		{
		case TASK_COND_LOGIN_AT_9_BETWEEN_10:
		{
			if (timeNow.tm_hour == 9)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_LOGIN_AT_12_BETWEEN_13:
		{
			if (timeNow.tm_hour == 12)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_LOGIN_AT_18_BETWEEN_19:
		{
			if (timeNow.tm_hour == 18)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_LOGIN_AT_21_BETWEEN_22:
		{
			if (timeNow.tm_hour == 21)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		default: break;
		}

		if (bUpdate)
		{
			UpdateTask(it.second);
			bUpdate = false;
		}
	}*/
}

CT_VOID CUserTask::OnUserRecharge(CT_DWORD dwTotalRecharge)
{
	for (auto& it : m_mapUserTask)
	{
		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		if (pTaskMode == NULL)
		{
			continue;
		}

		if (pTaskMode->wCompleteType != TASK_COND_RECHARGE)
		{
			continue;
		}

		if (pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
		{
			continue;
		}

		if (it.second.bFinish)
			continue;

		it.second.dwCurrProgress = dwTotalRecharge;
		if (it.second.dwCurrProgress > pTaskMode->dwTotalProgress)
		{
			it.second.dwCurrProgress = pTaskMode->dwTotalProgress;
		}

		UpdateTask(it.second);
	}
}

CT_VOID CUserTask::OnUserAddScore(CT_WORD wGameRoomKind, CT_LONGLONG llAddScore)
{
	/*bool bUpdate = false;
	for (auto& it : m_mapUserTask)
	{
		if (it.second.bFinish)
			continue;

		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		if (pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
		{
			continue;
		}

		switch (pTaskMode->wCompleteType)
		{
		case TASK_COND_PRIMARY_ROOM_PLAY:
		{
			if (wGameRoomKind == PRIMARY_ROOM)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_MIDDLE_ROOM_PLAY:
		{
			if (wGameRoomKind == MIDDLE_ROOM)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_SENIOR_ROOM_PLAY:
		{
			if (wGameRoomKind == SENIOR_ROOM)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;
			}
		}
		break;
		case TASK_COND_ANY_ROOM_EARN_SCORE:
		{
			if (llAddScore > 0)
			{
				it.second.dwCurrProgress += (CT_DWORD)llAddScore;
				if (it.second.dwCurrProgress > pTaskMode->dwTotalProgress)
				{
					it.second.dwCurrProgress = pTaskMode->dwTotalProgress;
				}
				bUpdate = true;
			}
		}
		break;
		default:break;
		}

		if (bUpdate)
		{
			UpdateTask(it.second);
			bUpdate = false;
		}
	}*/
}

CT_VOID	CUserTask::OnUserInRank()
{
	/*for (auto& it : m_mapUserTask)
	{
		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		if (pTaskMode == NULL)
		{
			continue;
		}

		if (pTaskMode->wCompleteType != TASK_COND_IN_ANY_RANK)
		{
			continue;
		}

		if (pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
		{
			continue;
		}

		if (it.second.bFinish)
			continue;

		it.second.dwCurrProgress = pTaskMode->dwTotalProgress;

		UpdateTask(it.second);

		//只有一条排行榜任务
		break;
	}*/
}

CT_VOID CUserTask::OnUserPlayGame(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	bool bUpdate = false;
	for (auto& it : m_mapUserTask)
	{
		if (it.second.bFinish)
			continue;

		tagTaskModel* pTaskMode = CUserTaskModel::get_instance().GetTaskModel(it.first);
		if (!pTaskMode)
		{
			continue;
		}
		if (pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
		{
			continue;
		}

		switch (pTaskMode->wCompleteType)
		{
		case TASK_COND_IN_ROOM_PLAY:
		{
			if (pTaskMode->wGameID == wGameID && pTaskMode->wKindID == wKindID && pTaskMode->wRoomKindID == wRoomKindID)
			{
				it.second.dwCurrProgress += 1;
				bUpdate = true;

				//完成任务的时候，增加对应玩家公会的贡献度
				CT_DWORD dwGroupID = m_pUser->GetGroupID();
				if (dwGroupID != 0 && pTaskMode->dwTotalProgress == it.second.dwCurrProgress)
                {
                    CGroupMgr::get_instance().UpdateGroupContribution(dwGroupID, 1);
                }
			}
		}
		break;
		default:break;
		}

		if (bUpdate)
		{
			UpdateTask(it.second);
			bUpdate = false;
			break;
		}
	}

	
}

CT_VOID CUserTask::InsertNewTask(const tagTaskModel* pTaskInfo)
{
	auto it = m_mapUserTask.find(pTaskInfo->wTaskID);
	if (it == m_mapUserTask.end())
	{
		tagUserTask userTask(pTaskInfo->wTaskID);
		m_mapUserTask.insert(std::make_pair(pTaskInfo->wTaskID, userTask));
		InsertTaskTodb(userTask);
	}
}

CT_VOID CUserTask::InsertTask(tagUserTask& userTask)
{
	auto it = m_mapUserTask.find(userTask.wTaskID);
	if (it == m_mapUserTask.end())
	{
		m_mapUserTask.insert(std::make_pair(userTask.wTaskID, userTask));
	}
}

CT_VOID CUserTask::ResetTask(CT_WORD wTaskID)
{
	auto it = m_mapUserTask.find(wTaskID);
	if (it != m_mapUserTask.end())
	{
		it->second.Reset();
		UpdateTask(it->second);
	}
}

CT_VOID CUserTask::ResetTaskInMidnight(CT_WORD wTaskID)
{
	auto it = m_mapUserTask.find(wTaskID);
	if (it != m_mapUserTask.end())
	{
		it->second.Reset();
		UpdateTaskToClient(it->second);
		UpdateTaskToCache(it->second);
	}
}

CT_VOID CUserTask::InsertTaskTodb(tagUserTask& userTask)
{
	UpdateTaskToCache(userTask);
	MSG_CS2DB_Insert_Task insertTask;
	insertTask.wTaskID = userTask.wTaskID;
	insertTask.dwUserID = m_pUser->GetUserID();

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_TASK, &insertTask, sizeof(insertTask));
}

CT_VOID	CUserTask::UpdateTask(tagUserTask& userTask)
{
	UpdateTaskToClient(userTask);
	UpdateTaskToCache(userTask);
	UpdateTaskTodb(userTask);
}

CT_VOID CUserTask::UpdateTaskToClient(tagUserTask& userTask)
{
	CT_DWORD dwUserID = m_pUser->GetUserID();
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	tagTaskModel* pTaskModel = CUserTaskModel::get_instance().GetTaskModel(userTask.wTaskID);
	if (pTaskModel == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[1024] = { 0 };
	CT_DWORD dwSendSize = 0;
	
	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_TASK_MAIN;
	msgHead.dwSubID = SUB_SC_TASK_PROGRESS;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	MSG_SC_Task_Progress taskProgress;
	taskProgress.wTaskID = userTask.wTaskID;
	taskProgress.dwCurrProgress = userTask.dwCurrProgress;
	taskProgress.cbFinish = userTask.bFinish;
	taskProgress.dwTotalProgress = pTaskModel->dwTotalProgress;
	memcpy(szBuffer+dwSendSize, &taskProgress, sizeof(taskProgress));
	dwSendSize += sizeof(MSG_SC_Task_Progress);

	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

CT_VOID CUserTask::UpdateTaskTodb(tagUserTask& userTask)
{
	MSG_CS2DB_Update_Task updateTask;
	updateTask.dwUserID = m_pUser->GetUserID();
	updateTask.wTaskID = userTask.wTaskID;
	updateTask.dwCurrProgress = userTask.dwCurrProgress;
	updateTask.dwReceiveTime = userTask.dwReceiveTime;
	updateTask.bFinish = userTask.bFinish;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_TASK, &updateTask, sizeof(updateTask));
}

CT_VOID CUserTask::UpdateTaskToCache(tagUserTask& userTask)
{
	m_pRedis->clear();

	acl::string key;
	key.format("task_%u", m_pUser->GetUserID());

	acl::string name;
	name.format("%u", userTask.wTaskID);

	acl::string value;
	value.format("%u,%u,%u,%d", userTask.wTaskID, userTask.dwCurrProgress, userTask.dwReceiveTime, userTask.bFinish);

	if (m_pRedis->hset(key.c_str(), name.c_str(), value.c_str()) == -1)
	{
		LOG(WARNING) << "set user task fail. task id:" << userTask.wTaskID;
	}
}

CT_VOID CUserTask::InsertTaskRewardTodb(CT_DWORD dwUserID, CT_WORD wTaskID, CT_LONGLONG llReward)
{
	MSG_CS2DB_Insert_TaskReward taskReward;
	taskReward.dwUserID = dwUserID;
	taskReward.wTaskID = wTaskID;
	taskReward.llReward = llReward;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_TASK_REWARD, &taskReward, sizeof(taskReward));
}

CT_VOID CUserTask::GetTaskReward(CT_WORD wTaskID)
{
	if (m_pUser == NULL)
	{
		return;
	}

	CT_DWORD dwUserID = m_pUser->GetUserID();
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	auto it = m_mapUserTask.find(wTaskID);
	if (it == m_mapUserTask.end())
	{
		return;
	}

	tagTaskModel* pTaskModel = CUserTaskModel::get_instance().GetTaskModel(wTaskID);
	if (pTaskModel == NULL)
	{
		return;
	}

	MSG_SC_Task_Reward_Result rewardResult;
	rewardResult.dwMainID = MSG_TASK_MAIN;
	rewardResult.dwSubID = SUB_SC_TASK_REWARD_RESULT;
	rewardResult.dwValue2 = dwUserID;

	if (it->second.dwCurrProgress != pTaskModel->dwTotalProgress)
	{
		rewardResult.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &rewardResult, sizeof(rewardResult));
		return;
	}

	if (pTaskModel->wTaskType == TASK_TYPE_DAILY)
	{
		ResetTask(wTaskID);
	}
	else
	{
		it->second.bFinish = true;
		//更新任务数据
		UpdateTask(it->second);
	}

	//金币变化记录
	CT_LONGLONG llRewardScore = pTaskModel->dwReward*TO_LL;
	CT_LONGLONG llSourceScore = CUserMgr::get_instance().GetUserScore(dwUserID);
	CT_LONGLONG llSourceBankScore = CUserMgr::get_instance().GetUserBankScore(dwUserID);
	CUserMgr::get_instance().InsertUserScoreChangeRecord(dwUserID, llSourceBankScore, llSourceScore, 0, llRewardScore, TASK_REWARD);

	//给玩家发送奖励
	CUserMgr::get_instance().AddUserScore(dwUserID, llRewardScore);

	//插入领奖记录
	InsertTaskRewardTodb(dwUserID, wTaskID, llRewardScore);

	//返回结果
	rewardResult.cbResult = 0;
	rewardResult.wTaskID = wTaskID;
	rewardResult.dwReward = pTaskModel->dwReward;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &rewardResult, sizeof(rewardResult));

	
}

CT_VOID CUserTask::SendAllTaskToClient()
{
	if (m_pUser == NULL)
	{
		return;
	}

	CT_DWORD dwUserID = m_pUser->GetUserID();
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;
	//任务信息
	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_TASK_MAIN;
	msgHead.dwSubID = SUB_SC_TASK_INFO;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	const MapTaskModel& taskModel = CUserTaskModel::get_instance().GetAllTaskModel();
	for (auto& it : taskModel)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Task_Info)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_TASK_MAIN;
			msgHead.dwSubID = SUB_SC_TASK_INFO;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		static MSG_SC_Task_Info taskInfo;
		taskInfo.wTaskID = it.second.wTaskID;
		taskInfo.wTaskType = it.second.wTaskType;
		taskInfo.wGameID = it.second.wGameID;
		taskInfo.wKindID = it.second.wKindID;
		taskInfo.wRoomKindID = it.second.wRoomKindID;
		taskInfo.dwTotalProgress = it.second.dwTotalProgress;
		taskInfo.dwReward = it.second.dwReward;

		memcpy(szBuffer + dwSendSize, &taskInfo, sizeof(MSG_SC_Task_Info));
		dwSendSize += sizeof(MSG_SC_Task_Info);
	}

	//发送
	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}

	//任务进度
	dwSendSize = 0;
	msgHead.dwMainID = MSG_TASK_MAIN;
	msgHead.dwSubID = SUB_SC_TASK_PROGRESS;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	for (auto& it : m_mapUserTask)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Task_Progress)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_TASK_MAIN;
			msgHead.dwSubID = SUB_SC_TASK_PROGRESS;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		tagTaskModel* pTaskModel = CUserTaskModel::get_instance().GetTaskModel(it.second.wTaskID);
		if (pTaskModel)
		{
			static MSG_SC_Task_Progress taskProgress;
			memset(&taskProgress, 0, sizeof(taskProgress));
			taskProgress.wTaskID = it.second.wTaskID;
			taskProgress.cbFinish = it.second.bFinish;
			taskProgress.dwCurrProgress = it.second.dwCurrProgress;
			taskProgress.dwTotalProgress = pTaskModel->dwTotalProgress;

			memcpy(szBuffer + dwSendSize, &taskProgress, sizeof(MSG_SC_Task_Progress));
			dwSendSize += sizeof(MSG_SC_Task_Progress);
		}
	}

	//发送
	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

////////////////////////////////////////////////////////////////////////
CUserTaskModel::CUserTaskModel()
{

}

CUserTaskModel::~CUserTaskModel()
{

}

CT_VOID  CUserTaskModel::InsertTaskModel(tagTaskModel* pTaskInfo)
{
	if (m_mapTaskModel.find(pTaskInfo->wTaskID) != m_mapTaskModel.end())
	{
		return;
	}

	tagTaskModel taskInfo;
	memcpy(&taskInfo, pTaskInfo, sizeof(taskInfo));

	m_mapTaskModel.insert(std::make_pair(taskInfo.wTaskID, taskInfo));
}

tagTaskModel* CUserTaskModel::GetTaskModel(CT_WORD wTaskID)
{
	auto it = m_mapTaskModel.find(wTaskID);
	if (it != m_mapTaskModel.end())
	{
		return &it->second;
	}

	return NULL;
}

const MapTaskModel& CUserTaskModel::GetAllTaskModel()
{
	return m_mapTaskModel;
}

CT_VOID CUserTaskModel::AcrossTheDay()
{
	for (auto& it : m_mapTaskModel)
	{
		switch (it.second.wTaskType)
		{
		case TASK_TYPE_DAILY:
		case TASK_TYPE_LOGIN:
		{
			MapGlobalUser& mapUser = CUserMgr::get_instance().GetAllGlobalUser();
			for (auto& itUser : mapUser)
			{
				std::unique_ptr<CUserTask>& userTask = itUser.second->GetUserTask();
				if (userTask.get())
				{
					//userTask->ResetTask(it.first);
					userTask->ResetTaskInMidnight(it.first);
				}
			}
		}
		break;
		default:
			break;
		}
	}
	//目前所有任务都是日常任务，凌晨统一到数据库清空数据，注意如果以后有其他类型的任务，就不能统一清理
	//ResetDailyTaskInDB();
}

CT_VOID CUserTaskModel::AcrossTheWeek()
{
	for (auto& it : m_mapTaskModel)
	{
		switch (it.second.wTaskType)
		{
		case TASK_TYPE_WEEKLY:
		{
			MapGlobalUser& mapUser = CUserMgr::get_instance().GetAllGlobalUser();
			for (auto& itUser : mapUser)
			{
				std::unique_ptr<CUserTask>& userTask = itUser.second->GetUserTask(); 
				if (userTask.get())
				{
					userTask->ResetTask(it.first);
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

CT_VOID CUserTaskModel::ResetDailyTaskInDB()
{
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_RESET_TASK);
}