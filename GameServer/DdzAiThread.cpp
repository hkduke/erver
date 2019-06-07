

#include "DdzAiThread.h"
#include "AndroidUserMgr.h"
#include "glog_wrapper.h"
#include "ServerCfg.h"

#ifdef _OS_LINUX_CODE
ddz_ai_interface* g_pDdzAi = 0;
void get_result(int event, int handle, const pai_interface_move* move)
{
	//Log_Msg(Log_Debug, "event=%d handle=%d-------------->type=%d alone1=%d alone2=%d combo_count=%d", event, handle, move->_type, move->_alone_1, move->_alone_2, move->_combo_count);

	if (event != pai_event_play_card && event != pai_event_play_card_passive)				//主动出牌
	{
		char szErrorMsg[128] = { 0 };
		_snprintf_info(szErrorMsg, sizeof(szErrorMsg), "event=%d handle=%d-------------->type=%d alone1=%d alone2=%d combo_count=%d", event, handle, move->_type, move->_alone_1, move->_alone_2, move->_combo_count);
		LOG(ERROR) << szErrorMsg;
		
		return;
	}

	CDdzAiThread* pDdzAiThread = CAndroidUserMgr::get_instance().GetDdzAiThread();
	if (pDdzAiThread)
	{
		std::shared_ptr<ai_action> aiActionPtr = pDdzAiThread->GetFreeAiActionPtr();

		std::map<unsigned int, ai_user>& MapHandleForUser = pDdzAiThread->GetHandleForUserMap();
		ai_user& aiUser = MapHandleForUser[handle];
		unsigned int uUserID = aiUser.uUserID;
		aiActionPtr->uUserID = uUserID;
		aiActionPtr->event = event;
		aiActionPtr->uTime = aiUser.uEventTime;

		//pai move
		aiActionPtr->_type = move->_type;
		aiActionPtr->_alone_1 = move->_alone_1;
		aiActionPtr->_alone_2 = move->_alone_2;
		aiActionPtr->_alone_3 = move->_alone_3;
		aiActionPtr->_alone_4 = move->_alone_4;
		aiActionPtr->_airplane_pairs = move->_airplane_pairs;
		for (int i = 0; i < 20; i++)
		{
			aiActionPtr->_combo_list[i] = move->_combo_list[i];
		}
		aiActionPtr->_combo_count = move->_combo_count;

		pDdzAiThread->InsertAiAction(aiActionPtr);
		MapHandleForUser.erase(uUserID);
	}
}
#endif

CDdzAiThread::CDdzAiThread()
{
}

CDdzAiThread::~CDdzAiThread()
{
}

void * CDdzAiThread::run()
{
#ifdef _OS_LINUX_CODE
	Log_Init("DdzAi");


	g_pDdzAi = new ddz_ai_interface;
	if (!g_pDdzAi)
	{
		Log_Msg(Log_Failed, "DdzAi: Create DdzAi Failed!\n");
		return 0;
	}

	acl::string strCfgPath;
	strCfgPath.format("./script/DdzAi_%d.cfg", CServerCfg::m_nServerID);
	if (g_pDdzAi->Initialize(strCfgPath.c_str()) != 0)
	{
		Log_Msg(Log_Failed, "DdzAi: Initialize() Failed!\n");
		return 0;
	}

	g_pDdzAi->SetPlayCardInterface(get_result);

	Log_Msg(Log_Normal, "Started.\n");


	while (1)
	{
		g_pDdzAi->Update();

		
		if (m_AiEventQue.GetSize() != 0)
		{
			std::shared_ptr<ai_event> aiEevent = m_AiEventQue.GetHead();
			unsigned int handle = g_pDdzAi->CreateAi();
			if (aiEevent->event == pai_event_play_card)
			{
			    ai_user aiUser;
			    aiUser.uUserID = aiEevent->uUserID;
			    aiUser.uEventTime = aiEevent->uTime;
				m_mapAiHandleForUser.insert(std::make_pair(handle, aiUser));
				//LOG(WARNING) << "recv play card data";
				g_pDdzAi->AiPlayCard(handle, aiEevent->pai_list, aiEevent->pai_list2, aiEevent->pai_list3, aiEevent->landlord);
			}
			else if (aiEevent->event == pai_event_play_card_passive)
			{
			    ai_user aiUser;
			    aiUser.uUserID = aiEevent->uUserID;
			    aiUser.uEventTime = aiEevent->uTime;
				m_mapAiHandleForUser.insert(std::make_pair(handle, aiUser));
				//LOG(WARNING) << "recv play card passive data, banker:"<< (int)aiEevent->landlord << ", out index : " << (int)aiEevent->outPaiIndex;
				g_pDdzAi->AiPlayCardPassive(handle, aiEevent->pai_list, aiEevent->pai_list2, aiEevent->pai_list3, aiEevent->landlord, aiEevent->outPaiIndex, &aiEevent->limit_card);
			}
			else if (aiEevent->event == pai_event_grab_landlord)
			{
				int landlord = g_pDdzAi->GrabLandlord(handle, aiEevent->pai_list, aiEevent->pai_list2, aiEevent->pai_list3, 0, aiEevent->grab_pai);

				//处理叫地主消息
				std::shared_ptr<ai_action> aiActionPtr = GetFreeAiActionPtr();

				aiActionPtr->uUserID = aiEevent->uUserID;;
				aiActionPtr->event = pai_event_grab_landlord;
				aiActionPtr->callLandScore = landlord;
				//LOG(WARNING) << "recv call banker data, call score: " << landlord;
				m_AiActionQue.PushBack(aiActionPtr);
			}

			//用过的指针插入到空闲对列里面
			aiEevent->reset();
			m_AiEventFreeQue.PushBack(aiEevent);

			//LOG(WARNING) << "running....";
		}
	}

	if (g_pDdzAi)
	{
		delete g_pDdzAi;
		g_pDdzAi = 0;
	}
#endif

	delete this;
	return NULL;
}


#ifdef _OS_LINUX_CODE
void CDdzAiThread::InsertAiEvent(std::shared_ptr<ai_event>& aiEvent)
{
	m_AiEventQue.PushBack(aiEvent);
}

std::shared_ptr<ai_event> CDdzAiThread::GetFreeAiEeventPtr()
{
	if (m_AiEventFreeQue.GetSize() == 0)
	{
		return std::make_shared<ai_event>();
	}

	return m_AiEventFreeQue.GetHead();
}

void CDdzAiThread::InsertAiAction(std::shared_ptr<ai_action>& aiAction)
{
	m_AiActionQue.PushBack(aiAction);
}

std::shared_ptr<ai_action> CDdzAiThread::GetOneAiActionPtr()
{
	return m_AiActionQue.GetHead();
}

std::shared_ptr<ai_action> CDdzAiThread::GetFreeAiActionPtr()
{
	if (m_AiActionFreeQue.GetSize() == 0)
	{
		return std::make_shared<ai_action>();
	}

	return m_AiActionFreeQue.GetHead();
}

void	CDdzAiThread::InsertFreeAiActionPtr(std::shared_ptr<ai_action>& aiAction)
{
	m_AiActionFreeQue.PushBack(aiAction);
}
#endif