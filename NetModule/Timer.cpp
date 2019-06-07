#include "Timer.h"
#include "NetModule.h"
#include "Utility.h"

extern CCObjPool<CMesBlock>	g_MsgBlockPool;
void CTimer::timer_callback(unsigned int id)
{
	switch (id)
	{
	case IDI_TIMER_LOOP:
		CNetModule::getSingletonPtr()->OnEventLoop();
		break;
	case IDI_TIMER_RECONNECT_SERVER:
		CNetModule::getSingletonPtr()->OnEventConnectServer();
		break;
	case IDI_TIMER_EVENT:
		{
			_AUTOMUTEX(&m_mutex);
			if (!m_waitAddTimeMap.empty())
			{
				for (auto& it : m_waitAddTimeMap)
				{
					if (m_TimeMap.find(it.first) == m_TimeMap.end())
					{
						TimerEvent tv;
						tv.id = it.second.id;
						tv.elapse = it.second.elapse;
						tv.lastTime = it.second.lastTime;
						tv.bRepeats = it.second.bRepeats;
						tv.pThread = it.second.pThread;
						m_TimeMap.insert(std::make_pair(tv.id, tv));
					}
				}
				m_waitAddTimeMap.clear();
			}

			for (auto iter = m_TimeMap.begin(); iter != m_TimeMap.end(); )
			{
				TimerEvent& tv = iter->second;
				if (tv.elapse > 0)
				{
					CT_DWORD tick = Utility::GetTick();
					if (tick - tv.lastTime < tv.elapse)
					{
						iter++;
						continue;
					}
					CMesBlock* pBlock = g_MsgBlockPool.NewObj();
					if (pBlock == NULL)
					{
						LOG(ERROR) << "get a null message block.";
						continue;
					}
					CT_CHAR* pBuf = pBlock->Allocate(sizeof(CT_DWORD));
					if (pBuf == NULL)
					{
						LOG(ERROR) << "Allocate message block buf fail.";
						continue;
					}
					pBlock->m_byMsgTag = MSC_S_HANDLE_TIMER;
					pBlock->m_pSocketStream = NULL;
					memcpy(pBlock->m_pBuf, &(tv.id), sizeof(CT_DWORD));
					CWorkThread* pWorkThread = ((CWorkThread*)(tv.pThread));
					if (NULL != pWorkThread)
					{
						pWorkThread->InsertMseBlock(pBlock);
					}
					if (!tv.bRepeats)
					{
						iter = m_TimeMap.erase(iter);
					}
					else
					{
						tv.lastTime = tick;
						iter++;
					}
				}
			}
		}
		break;
	//case IDI_TIMER_RECONNECT_SERVER:
	//	{
	//		CNetModule::getSingleton().ReconnectServer();
	//	}
	//	break;

	case IDI_TIMER_HEART_BEAT:
	{
		CNetModule::getSingleton().OnSendHeartBeat();
	}
	break;
	default:
		break;
	}
}

CT_BOOL	CTimer::SetTimer(CT_DWORD timerId, CT_DWORD elapse, CThread* pThread, CT_BOOL bRepeat)
{
	_AUTOMUTEX(&m_mutex);
	TIMEITEMMAP::iterator iter = m_TimeMap.find(timerId);
	TIMEITEMMAP::iterator iter2 = m_waitAddTimeMap.find(timerId);
	if (iter == m_TimeMap.end() && iter2 == m_waitAddTimeMap.end())
	{
		TimerEvent tv;
		tv.id = timerId;
		tv.elapse = elapse;
		tv.lastTime = Utility::GetTick();
		tv.bRepeats = bRepeat;
		tv.pThread = pThread;

		m_waitAddTimeMap.insert(std::make_pair(timerId, tv));
		return true;
	}

	return false;
}

CT_VOID CTimer::KillTimer(CT_DWORD timerId)
{
	_AUTOMUTEX(&m_mutex);

	TIMEITEMMAP::iterator iter = m_TimeMap.find(timerId);
	if (iter != m_TimeMap.end())
	{
		m_TimeMap.erase(iter);
	}
}