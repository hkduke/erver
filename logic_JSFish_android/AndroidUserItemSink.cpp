
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include <algorithm>
#include <random>
#include <limits.h>
//////////////////////////////////////////////////////////////////////////

//辅助时间
#define TIME_LESS						2									//最少时间
#define TIME_DELAY_TIME					3									//延时时间

//游戏时间
#define TIME_USER_ADD_SCORE				5									//下注时间
#define TIME_RUSH						3									//火拼
#define TIME_OPEN_CARD					3									//开牌时间

//游戏时间
#define IDI_USER_ADD_SCORE				(101)								//下注定时器
#define IDI_RUSH						(102)								//火拼定时器
#define IDI_OPEN_CARD					(103)								//赢家开牌消息
#define MAX_LOCK_COUNT					20
//////////////////////////////////////////////////////////////////////////

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
	//接口变量
	srand((unsigned)time(NULL));

}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{

}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
}


bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID)
	{
		case IDI_ANDROID_AUTO_SHOOT:
		{
			if(m_mapAliveBird.size() == 0)
			{
				m_nLockTimers = 0;
				m_nLockBirdID = 0;
				return CT_FALSE;
			}

			if(m_nLockTimers > MAX_LOCK_COUNT)
			{
				m_nLockTimers = 0;
				m_nLockBirdID = 0;
			}

			if(m_nLockTimers == 0 && m_nLockBirdID == 0)
			{
				//随机挑一个
				size_t s = m_mapAliveBird.size();
				srand(time(NULL));
				s = rand() % s;

				size_t loop = 0;
				for(std::map<unsigned int ,CMD_S_Send_Bird>::iterator it = m_mapAliveBird.begin(); it != m_mapAliveBird.end() && loop <= s; it++, loop++)
				{
					if(loop == s)
					{
						m_nLockTimers  = 0;
						m_nLockBirdID = it->first;
						break;
					}
				}
			}

			if(m_nLockBirdID != 0 && m_self_score >= cannon_mulriple_[4])
			{
				//m_self_score -= cannon_mulriple_[4];
				m_nLockTimers++;
				CMD_C_Fire fire;
				fire.bullet_mulriple_  = cannon_mulriple_[4];
				fire.lock_bird_id_ = m_nLockBirdID;
				fire.chair_id_ = m_self_chair_id;
				fire.rote_ = 90.0f;
				fire.bullet_type_ = 0;
				fire.bullet_type_ |= BULLET_PENETRATE;
				fire.bullet_id = m_bullet_id_cur;
				m_bullet_id_cur++;
				if(m_bullet_id_cur > m_bullet_id_end)
				{
					m_bullet_id_cur = m_bullet_id_start;
				}
				m_pTableFrame->OnGameEvent(m_self_chair_id, SUB_C_FIRE, &fire, sizeof(fire));
				//LOG(ERROR) << "机器人:"<<m_self_chair_id << "发送炮弹,倍率:"<<fire.bullet_mulriple_ << " 锁定鱼ID:" <<fire.lock_bird_id_ << " 当前鱼数:"<<m_mapAliveBird.size();
			}
		}
		break;
		case IDI_CHECK_TIMEOUT_FISH:
		{
			for( std::map<unsigned int , time_t>::iterator it = m_mapBirdCreatetime.begin(); it != m_mapBirdCreatetime.end(); it++)
			{
				if(time(NULL) - it->second > 3 * 60)
				{
					std::map<unsigned int ,CMD_S_Send_Bird>::iterator itFish = m_mapAliveBird.find(it->first);
					if(itFish != m_mapAliveBird.end())
					{
						if(m_nLockBirdID == itFish->first)
						{
							m_nLockBirdID = 0;
							m_nLockTimers = 0;
						}
						m_mapAliveBird.erase(itFish);
					}
				}
			}
		}
		break;
	}

	return CT_TRUE;
}

bool CAndroidUserItemSink::OnGameMessage( CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
	if(pData == NULL || wDataSize == 0)
	{
		return CT_FALSE;
	}

	switch (wSubCmdID)
	{
		case STATUSFREE:
		{
			CMD_S_StatusFree* cmsf = (CMD_S_StatusFree *)pData;
			for(int i = 0; i < MAX_CANNON_TYPE; i++)
			{
				cannon_mulriple_[i] = cmsf->cannon_mulriple_[i];
			}
		}
		break;
		case SUB_S_ENTER_ANDROID_INFO:
		{
			S_Enter_Android_info *info = (S_Enter_Android_info *)pData;
			m_self_chair_id = info->chair_id;
			m_self_score = info->score;
			m_nLockBirdID = 0;
			m_nLockTimers = 0;

			if(m_self_chair_id == 0)
			{
				m_bullet_id_start = 0;
				m_bullet_id_cur = 0;
				m_bullet_id_end = 200;
			}
			else if(m_self_chair_id == 1)
			{
				m_bullet_id_start = 201;
				m_bullet_id_cur = 201;
				m_bullet_id_end = 400;
			}
			else if(m_self_chair_id == 2)
			{
				m_bullet_id_start = 401;
				m_bullet_id_cur = 401;
				m_bullet_id_end = 600;
			}
			else if(m_self_chair_id == 3)
			{
				m_bullet_id_start = 601;
				m_bullet_id_cur = 601;
				m_bullet_id_end = 800;
			}
			m_pAndroidUserItem->SetTimer(IDI_ANDROID_AUTO_SHOOT, TIME_ANDROID_AUTO_SHOOT);
			m_pAndroidUserItem->SetTimer(IDI_CHECK_TIMEOUT_FISH, TIMER_CHECK_TIMEOUT_FISH);

			//LOG(ERROR) << "机器人:" << m_self_chair_id << "进入房间";
		}
		break;
		case SUB_S_SEND_BIRD:
		{
			CMD_S_Send_Bird *pBird = (CMD_S_Send_Bird *)pData;
			m_mapAliveBird[pBird->id_] = *pBird;
			m_mapBirdCreatetime[pBird->id_] = time(NULL);
		}
		break;
		case SUB_S_FIRE_FAILED:
		{
			CMD_S_Fire_Failed *csff = (CMD_S_Fire_Failed*)pData;
			if(csff->chair_id_ == m_self_chair_id)
			{
				m_self_score = csff->nowGlod;
			}
		}
		break;
		case SUB_S_SEND_BULLET:
		{
			CMD_S_Send_Bullet *cssb = (CMD_S_Send_Bullet*)pData;
			if(cssb->chair_id_ == m_self_chair_id)
			{
				m_self_score = cssb->cur_gold_;

				CMD_C_Catch_Fish cccf;
				cccf.chair_id = m_self_chair_id;
				cccf.bullet_kind = BULLET_PENETRATE;
				cccf.fish_id_ = m_nLockBirdID;
				cccf.bullet_multiple = cannon_mulriple_[4];
				cccf.bullet_id = cssb->bullet_id;
				m_pTableFrame->OnGameEvent(m_self_chair_id, SUB_C_CATCH_FISH, &cccf, sizeof(cccf));

				//LOG(ERROR) << "机器人:" <<m_self_chair_id << " 击中鱼:"<<cccf.fish_id_;
			}
		}
		break;
		case SUB_S_CATCH_BIRD:
		{
			Catch_Bird *pcb = (Catch_Bird*)pData;
			if(pcb->chair_id_ == m_self_chair_id)
            {
			    m_self_score = pcb->now_money;
			    //LOG(ERROR) << "机器人: "<<m_self_chair_id << " 杀死鱼:"<<pcb->bird_id_ << " 当前金币: "<<m_self_score;
            }

            std::map<unsigned int ,CMD_S_Send_Bird>::iterator it = m_mapAliveBird.find(pcb->bird_id_);
            if(it != m_mapAliveBird.end())
            {
                m_mapAliveBird.erase(it);
                if(it->first == m_nLockBirdID)
                {
                    m_nLockBirdID = 0;
                    m_nLockTimers = 0;
                }
            }
		}
		break;
	}

	return CT_TRUE;
}

bool CAndroidUserItemSink::ClearAllTimer()
{
	return true;
}
