
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include "../logic_red_packet_score/MSG_HB.h"
#include "DataQueue.h"
#include <algorithm>


#define IDI_ROBOT_OPEN_HB			400					//抢红包(此值必须从400开始，防止加上红包ID后与IDI_ROBOT_FA_HB的重复)
#define IDI_ROBOT_FA_HB				300					//发红包
#define TIME_IS_CAN_OUT_CARD		7					//是否能出牌

#define PASSCARD_TIME(A) (((A) < 2)? 2:(A))

bool complare(unsigned char a, unsigned char b)
{
	return a > b;
}

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
	srand(time(NULL));
	m_dwRandIndex = 0;
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	m_pTableFrame = NULL;
	m_wChairID = INVALID_CHAIR;
	m_pAndroidUserItem = NULL;
}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
    ReadJsonCfg();
}

bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{

    m_pAndroidUserItem->KillTimer(dwTimerID);
    dwTimerID -= (dwParam%300);
	switch (dwTimerID)
	{
	case IDI_ROBOT_OPEN_HB:
	{
		CMD_C_OpenHB outCard;
		memset(&outCard, 0, sizeof(outCard));
		outCard.wChairID = m_wChairID;
		outCard.dwHBID = dwParam;
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_OPEN_HB, &outCard, sizeof(outCard));
		//LOG(INFO) << "ontimer IDI_ROBOT_OPEN_HB time";
	}
	break;
	case IDI_ROBOT_FA_HB:
	{
		int iRand = RAND(0,100,m_dwRandIndex++);
		CMD_C_ApplyBatchFaHB outCard;
		memset(&outCard, 0, sizeof(outCard));
		for (CT_DWORD i = 0; i < m_vRobotFaHBAmountCfg.size(); ++i)
		{
			if(dwParam != (CT_DWORD)m_vRobotFaHBAmountCfg[i].iHBAllotCount)
			{
				continue;
			}
            int iTotal = 0;
            for (CT_DWORD j = 0; j < m_vRobotFaHBAmountCfg[i].vFaHBAmount.size(); ++j)
			{
                iTotal += m_vRobotFaHBAmountCfg[i].vFaHBAmount[j].iProba;
				if(iRand < iTotal)
				{
					outCard.iHBAmount = (10*RAND(m_vRobotFaHBAmountCfg[i].vFaHBAmount[j].iHBMinAmountMultipleLowerLimit, m_vRobotFaHBAmountCfg[i].vFaHBAmount[j].iHBMinAmountMultipleUpperLimit, m_dwRandIndex++))+ m_pTableFrame->GetGameKindInfo()->dwEnterMinScore*TO_DOUBLE;
					if(outCard.iHBAmount > (m_pTableFrame->GetGameKindInfo()->dwEnterMaxScore*TO_DOUBLE))
					{
						outCard.iHBAmount = (m_pTableFrame->GetGameKindInfo()->dwEnterMaxScore*TO_DOUBLE);
					}
					break;
				}
			}
		}
		if(0==outCard.iHBAmount)
		{
			outCard.iHBAmount = 10;
			LOG(ERROR) << "m_vRobotFaHBAmountCfg error";
		}
		outCard.wChairID = m_wChairID;
		outCard.cbThunderNO = RAND_NUM(m_dwRandIndex++)%10;
		outCard.iFaHBCount = 1;
		outCard.iHBAllotCount = dwParam;
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_APPLY_BATCH_FA_HB, &outCard, sizeof(outCard));
	}
	break;
	default:
		break;
	}
	return true;
}

bool CAndroidUserItemSink::OnGameMessage( CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_NOTICE_ROBOT_GRAB_HB:
	{
		return OnSubGameStart(pData, wDataSize);
	}
	case SUB_S_NOTICE_ROBOT_FA_HB:
	{
		return OnSubFaHB(pData, wDataSize);
	}
	case SUB_S_GAME_CONCLUDE:
	{
		return OnGameOver(pData, wDataSize);
	}
	default: break;
	}
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_NoticeRobotGrabHB))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_NoticeRobotGrabHB* pGameStart = (CMD_S_NoticeRobotGrabHB*)pBuffer;
	if (!pGameStart)
	{
	    LOG(ERROR) << "pGameStart==NULL";
        return false;
    }

	if(pGameStart->wChairID != m_wChairID)
    {
        return true;
    }

	if(pGameStart->wGrabTime <= 0 || pGameStart->wGrabTime >= pGameStart->wTimeLeft)
	{
		pGameStart->wGrabTime = RAND_NUM(m_dwRandIndex++)%4;
	}

	m_pAndroidUserItem->SetTimer(IDI_ROBOT_OPEN_HB+(pGameStart->dwHBID%300), pGameStart->wGrabTime * 1000, pGameStart->dwHBID);
	//LOG(INFO) << "set IDI_ROBOT_OPEN_HB time: "<< pGameStart->wGrabTime * 1000;

	return true;
}
bool CAndroidUserItemSink::OnSubFaHB(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_NoticeRobotFaHB))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_NoticeRobotFaHB* pNetInfo = (CMD_S_NoticeRobotFaHB*)pBuffer;
	if (!pNetInfo)
	{
		LOG(ERROR) << "pGameStart==NULL";
		return false;
	}

	if(pNetInfo->wChairID != m_wChairID)
    {
        return true;
    }

	m_pAndroidUserItem->SetTimer(IDI_ROBOT_FA_HB+pNetInfo->iHBAllotCount, (RAND_NUM(m_dwRandIndex++)%4+1) * 1000, pNetInfo->iHBAllotCount);
	return true;
}
bool CAndroidUserItemSink::OnGameOver(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}
bool CAndroidUserItemSink::ReadFile(std::string & file, std::string & str)
{
	std::ifstream in;
	std::string line;

	in.open(file, std::ifstream::in);
	if (!in.is_open())
		return false;
	while (getline(in, line)) {
		str.append(line + "\n");
	}
	in.close();
	return true;
}
void CAndroidUserItemSink::ReadJsonCfg()
{
    std::string file;
    if(m_pTableFrame && m_pTableFrame->GetGameKindInfo() && SENIOR_ROOM==m_pTableFrame->GetGameKindInfo()->wRoomKindID)
    {
        file="./script/gameserver_HBSL_wxhb.json";
    }
    else
    {
        file="./script/gameserver_HBSL.json";
    }
	std::string data;
	ReadFile(file,data);
	acl::json json;
	json.update(data.c_str());

	//发红包金额配置
    if(!json.getElementsByTagName("RobotFaHBAmountCfg").empty())
    {
        m_vRobotFaHBAmountCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("RobotFaHBAmountCfg")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            GS_RobotFaHBAmountCfg RobotFaHBAmountCfg;
            RobotFaHBAmountCfg.iHBAllotCount = 0;
            acl::json jsonTmp(*node2);

            if(jsonTmp.getElementsByTagName("iHBAllotCount").size() > 0)
            {
                RobotFaHBAmountCfg.iHBAllotCount = (CT_INT32)(*(jsonTmp.getElementsByTagName("iHBAllotCount")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("vRobotFaHBAmountTable").size() > 0)
            {
                acl::json_node *node3 = jsonTmp.getElementsByTagName("vRobotFaHBAmountTable")[0]->first_child();
                acl::json_node *node4 = node3->first_child();
                int iTotalProba = 0;
                while(node3 != NULL && node4 != NULL)
                {
                    GS_RobotFaHBAmountTable RobotFaHBAmountTable;
                    memset(&RobotFaHBAmountTable, 0, sizeof(RobotFaHBAmountTable));
                    acl::json jsonTmp2(*node4);
                    if(jsonTmp2.getElementsByTagName("iHBMinAmountMultipleLowerLimit").size() > 0)
                    {
                        RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iHBMinAmountMultipleLowerLimit")[0])->get_int64());
                        if(RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit < 0 || RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit > 29)
						{
							RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit = 0;
						}
                    }
					if(jsonTmp2.getElementsByTagName("iHBMinAmountMultipleUpperLimit").size() > 0)
					{
						RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iHBMinAmountMultipleUpperLimit")[0])->get_int64());
						if(RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit < 1 || RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit > 30)
						{
							RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit = 5;
						}
					}
					if(RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit < RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit)
					{
						CT_INT32 iTemp = RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit;
						RobotFaHBAmountTable.iHBMinAmountMultipleUpperLimit = RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit;
						RobotFaHBAmountTable.iHBMinAmountMultipleLowerLimit = iTemp;
					}
                    if(jsonTmp2.getElementsByTagName("iProba").size() > 0)
                    {
                        RobotFaHBAmountTable.iProba = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iProba")[0])->get_int64());
                    }
                    iTotalProba += RobotFaHBAmountTable.iProba;
                    node4 = node3->next_child();
                    RobotFaHBAmountCfg.vFaHBAmount.push_back(RobotFaHBAmountTable);
                }
                if(iTotalProba != 100)
                {
                    LOG(ERROR) << "vRobotFaHBAmountTable cfg error";
                }
            }
            node2 = node1->next_child();
            m_vRobotFaHBAmountCfg.push_back(RobotFaHBAmountCfg);
        }
    }

	return;
}