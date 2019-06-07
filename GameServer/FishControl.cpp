#include "FishControl.h"
#include "ServerCfg.h"
#include "FishServerCfg.h"
#include "GameTableManager.h"
#include "glog_wrapper.h"
#include <cmath>
#include "testlog.h"
#include "NetModule.h"
#include "timeFunction.h"
#include "AndroidUserMgr.h"
#include <math.h>
#include <algorithm>
extern CServerCfg* pSerCfg;
extern CFishServerCfg *pFishSerCfg;
extern CNetConnector *pNetDB;

CFishControl::CFishControl()
{
	m_llKuCunMostTop = 0;
	m_llKuCunMostLow = 0;
    m_llTestAllPlayerTotalTax = 0;
}


CFishControl::~CFishControl()
{
}

CT_VOID CFishControl::SinglePlayerMostWinLoseScore(CT_DWORD dwUserID, CT_LONGLONG addScore)
{
	if (m_mapTestPlayerMostWinLoseScore.find(dwUserID) == m_mapTestPlayerMostWinLoseScore.end())
	{
		m_mapTestPlayerMostWinLoseScore[dwUserID] = addScore;
	}
	else
	{
		m_mapTestPlayerMostWinLoseScore[dwUserID] = m_mapTestPlayerMostWinLoseScore[dwUserID] + addScore;
	}
}

CT_VOID CFishControl::TongJi(tagTongJiInfo &info)
{
	/*m_dwPlayerCnt += info.dwPlayerCnt;
	if (info.llWinMost > m_llWinMost)
	{
		m_llWinMost = info.llWinMost;
	}
	if (info.llLoseMost < m_llLoseMost)
	{
		m_llLoseMost = info.llLoseMost;
	}
	m_dwPoChanPlayerCnt += info.dwPoChanPlayerCnt;
	m_dwWinPlayerCnt += info.dwWinPlayerCnt;*/
    //m_llTestAllPlayerTotalTax += info.dwTax;

    if(info.type == TJ_PLAYER_SHOOT_FISH_INFO)
	{
    	//统计的是捕鱼信息就立即写数据库
		MSG_GS2DB_PlayerFishInfo pInfo;
		pInfo.nTotalShootCount = info.nTotalShootCount;
		pInfo.nTotalDeathCount = info.nTotalDeathCount;
		pInfo.gameIndex = info.gameIndex;
		pInfo.dwUserID = info.dwUserID;
		memcpy(pInfo.llPerFishTypeScore, info.llPerFishTypeScore, sizeof(info.llPerFishTypeScore));
		memcpy(pInfo.nPerFishTypeDeathCount, info.nPerFishTypeDeathCount, sizeof(info.nPerFishTypeDeathCount));
		memcpy(pInfo.nPerFishTypeShootCount, info.nPerFishTypeShootCount, sizeof(info.nPerFishTypeShootCount));
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_PLAYER_FISH_INFO, &pInfo, sizeof(MSG_GS2DB_PlayerFishInfo));
	}
	else if(info.type == TJ_PLAYER_Win_bankrupted_Pro)
	{
		//统计玩家的破产率胜率
		if(info.ePWBInfo == PWB_ENTER)
		{
			m_dwTodayTotalEnterCount++;
		}
		else if(info.ePWBInfo == PWB_EXIT_WIN)
		{
			m_dwTodayTotalPlayerCount_Win++;
		}
		else if(info.ePWBInfo == PWB_EXIT_Bankrup)
		{
			m_dwTodayTotalPlayerCount_Bankrupted++;
		}
		else if(info.ePWBInfo == PWB_EXIT_LOSE)
		{
			m_dwTodayTotalPlayerCount_Lose++;
		}
		else if(info.ePWBInfo == PWB_EXIT_NO_WIN_LOSE)
		{
			m_dwTodayTatalPlayerCount_NoWinLose++;
		}
		else if(info.ePWBInfo == PWB_FORBID_POCHAN_REWARD)
		{
			m_dwTodayForbidPoChanReward += info.llPoChanReward;
		}
		else if(info.ePWBInfo == PWB_BIGFISH_INFO)
        {
			std::map<CT_BYTE , sBigFishInFo>::iterator itBF = m_mapTodayBigFishInfo.find(info.bigFishInfo.cbFishType);
			if(itBF == m_mapTodayBigFishInfo.end())
			{
				sBigFishInFo bgInfo;
				bgInfo.llPlayerGetScore = info.bigFishInfo.llPlayerGetScore;
				bgInfo.llPlayerLostScore = info.bigFishInfo.llPlayerLostScore;
				m_mapTodayBigFishInfo[info.bigFishInfo.cbFishType] = bgInfo;
			}
			else
			{
				itBF->second.llPlayerLostScore += info.bigFishInfo.llPlayerLostScore;
				itBF->second.llPlayerGetScore += info.bigFishInfo.llPlayerGetScore;
			}
        }
	}
}

CT_VOID CFishControl::CheckBaseCapturePro()
{
	//库存波动图中线值
	int nMiddleValue = 0;

	int nCapturePro = 10000;
	CT_BOOL bFindPro = CT_FALSE;
	//CT_INT32 nStateValue = 0;
	//用现有的库存值到血池状态配置表中查找对应的基础捕获概率:倒序查找
	if (m_nAllTableKuCun > pFishSerCfg->m_arrayFishBloodPoolState[pFishSerCfg->m_arraySize - 1].StateValue + nMiddleValue)
	{
		nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[pFishSerCfg->m_arraySize - 1].CapturePro;
		bFindPro = CT_TRUE;
		//nStateValue = pSerCfg->m_arrayFishBloodPoolState[pSerCfg->m_arraySize - 1].StateValue;
	}
	else if (m_nAllTableKuCun < pFishSerCfg->m_arrayFishBloodPoolState[0].StateValue + nMiddleValue)
	{
		nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[0].CapturePro;
		bFindPro = CT_TRUE;
		//nStateValue = pSerCfg->m_arrayFishBloodPoolState[0].StateValue;
	}
	else
	{
		if (m_nAllTableKuCun > nMiddleValue)
		{
			for (int i = pFishSerCfg->m_arraySize - 1; i >= 0 && pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue > nMiddleValue; i--)
			{
				//if (pSerCfg->m_arrayFishBloodPoolState[i].StateValue > 0 && pSerCfg->m_arrayFishBloodPoolState[i - 1].StateValue < 0
				//	&& m_nAllTableKuCun < pSerCfg->m_arrayFishBloodPoolState[i].StateValue)
				//{
				//	nCapturePro = pSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
				//	bFindPro = CT_TRUE;
				//	break;
				//}

				if(pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro > 10000
					&& pFishSerCfg->m_arrayFishBloodPoolState[i - 1].CapturePro < 10000
					&& m_nAllTableKuCun < pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue
					&& m_nAllTableKuCun > nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}

				if (m_nAllTableKuCun == pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}
				else if (m_nAllTableKuCun < pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue
					&& m_nAllTableKuCun > pFishSerCfg->m_arrayFishBloodPoolState[i - 1].StateValue + nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i - 1].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}
			}
		}
		else if (m_nAllTableKuCun < nMiddleValue)
		{
			for (int i = 0; i < pFishSerCfg->m_arraySize && pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue < nMiddleValue; i++)
			{
				//if (pSerCfg->m_arrayFishBloodPoolState[i].StateValue < 0 && pSerCfg->m_arrayFishBloodPoolState[i + 1].StateValue > 0
				//	&& m_nAllTableKuCun > pSerCfg->m_arrayFishBloodPoolState[i].StateValue)
				//{
				//	nCapturePro = pSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
				//	bFindPro = CT_TRUE;
				//	break;
				//}

				if (pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro < 10000
					&& pFishSerCfg->m_arrayFishBloodPoolState[i + 1].CapturePro > 10000
					&& m_nAllTableKuCun < nMiddleValue
					&& m_nAllTableKuCun > pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}

				if (m_nAllTableKuCun == pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}
				else if (m_nAllTableKuCun > pFishSerCfg->m_arrayFishBloodPoolState[i].StateValue + nMiddleValue &&
					m_nAllTableKuCun < pFishSerCfg->m_arrayFishBloodPoolState[i + 1].StateValue + nMiddleValue)
				{
					nCapturePro = pFishSerCfg->m_arrayFishBloodPoolState[i + 1].CapturePro;
					bFindPro = CT_TRUE;
					break;
				}
			}
		}
		else
		{
			//库存为0,系统不亏不赚就设置捕获概率为10000,意味着此时玩家捕获鱼的概率不受血池控制,只受鱼的固定捕获概率控制。
			nCapturePro = 10000;
			bFindPro = CT_TRUE;
		}
	}

	//测试：
	if (nCapturePro != 10000)
	{
		if (nCapturePro > 10000)
		{
			//nCapturePro += 170;
		}
		else
		{
			//nCapturePro += 150;
		}
	}

	if (bFindPro == CT_FALSE)
	{
		//LOG(ERROR) << "当前库存值: " << m_nAllTableKuCun << " 在血池状态配置表中没有找到对应的状态值进行匹配";
		//WrtLog(1, "当前库存值: %d 没有匹配到状态值", m_nAllTableKuCun);
	}

	//测试，扩大基础捕获率的影响2倍
	/*
	if (nCapturePro > 10000)
	{
		int nTemp = nCapturePro - 10000;
		nTemp = nTemp * 2;
		nCapturePro = nTemp + 10000;
	}
	else if (nCapturePro < 10000)
	{
		int nTemp = 10000 - nCapturePro;
		nTemp = nTemp * 2;
		nCapturePro = 10000 - nTemp;
	}
	*/

	if (m_nAllTableKuCun > 0)
	{
		m_llTestZhengTotal += m_nAllTableKuCun;
		m_llTestZhengCnt++;
	}
	else if (m_nAllTableKuCun < 0)
	{
		m_llTestFuTotal += m_nAllTableKuCun;
		m_llTestFuCnt++;
	}

	std::map<int, CT_UINT32>::iterator itTest = m_mapTestCapProCnt.find(nCapturePro);
	if (itTest == m_mapTestCapProCnt.end())
	{
		m_mapTestCapProCnt[nCapturePro] = 1;
	}
	else
	{
		m_mapTestCapProCnt[nCapturePro] = m_mapTestCapProCnt[nCapturePro] + 1;
	}

	//根据服务器ID的不同设置不同的文件名，否则要出问题:三个场次的日志输出到同一个LOG文件中.
	//关闭日志
	//if(CServerCfg::)
	//关闭测试日志
	//WrtLog(CServerCfg::m_nServerID, 1, "%d", nCapturePro);
	//WrtLog(CServerCfg::m_nServerID * 10, 1, "%lld", m_nAllTableKuCun);
	//如果匹配不到就设置基础捕获概率为1,用鱼的固定捕获概率去影响玩家命中率
	CGameTableManager::get_instance().SetAllFishTableBaseCapPro(nCapturePro);

	if(nCapturePro > 10000)
	{
		m_dwTodayRewardCount++;
	}
	else if(nCapturePro < 10000)
	{
		m_dwTodayXiFenCount++;
	}

	std::map<CT_DWORD, CT_DWORD>::iterator itProInfo = m_mapTodayProInfo.find(nCapturePro);
	if(itProInfo == m_mapTodayProInfo.end())
	{
		m_mapTodayProInfo[nCapturePro] = 1;
	}
	else
	{
		m_mapTodayProInfo[nCapturePro] += 1;
	}

	if (bFindPro)
	{
		//LOG(ERROR) << "匹配到的状态值: " << nStateValue <<" 设置所有桌子	的基础捕获概率为: 万分之" << nCapturePro;
		//WrtLog(1, "当前库存值:%d 匹配到的状态值: %d  设置所有桌子的基础捕获概率为 :%d ", m_nAllTableKuCun,nStateValue, nCapturePro);
	}

	CT_UINT64 nCurTime = time(NULL);
	if (diffTimeDay(m_nTodayTime, nCurTime))
	{
		//写数据库 当日总吸分 当日总放分
		SaveFishTodayTotalWinLoseScore();

		//跨天重置数据,这些当日和其他当日数据不同,这些数据不需要在每次保存数据后重置.
		m_dwTodayTotalEnterCount = 0;
		m_dwTodayTotalPlayerCount_Win = 0;
		m_dwTodayTotalPlayerCount_Bankrupted = 0;
		m_dwTodayTotalPlayerCount_Lose = 0;
		m_dwTodayTatalPlayerCount_NoWinLose = 0;
		m_dwTodayForbidPoChanReward = 0;
		m_mapTodayBigFishInfo.clear();
		m_dwTodayRewardCount = 0;
		m_dwTodayXiFenCount = 0;
		m_mapTodayProInfo.clear();
	}

	if (m_bStartAddScore == CT_FALSE)
	{
		//每间隔5秒获取一次所有玩家最新输赢值
		TongJiAllPlayerAddScore();
	}
}

CT_BOOL SortAllUserAddScore(tagPlayerAddScore& a, tagPlayerAddScore& b)
{
	return a.llAddScore < b.llAddScore;
}

CT_VOID CFishControl::TongJiAllPlayerAddScore()
{
	//统计之前先把所有玩家的输赢清空
	m_vecUserAddScore.clear();
	m_vecCurrentDianKongWinUsers.clear();

	ITableFrame *pTable = NULL;
	ITableFrameSink *pTableSink = NULL;
	VecTableFrame AllTables = CGameTableManager::get_instance().GetTableFrame();
	for(size_t i=0;i<AllTables.size();i++)
	{
		pTable = AllTables[i];
		if (pTable)
		{
			pTableSink = pTable->GetTableFrameSink();
			if (pTableSink)
			{
				pTableSink->GetAllPlayerAddScore(m_vecUserAddScore);
			}
		}
	}

	//升序排列
	std::sort(m_vecUserAddScore.begin(), m_vecUserAddScore.end(), SortAllUserAddScore);

	CT_LONGLONG llFangFen = m_llSysDianKongWinLoseTotalScore;
	if (m_vecUserAddScore.size() > 0 && llFangFen > 0 && llFangFen > CFishServerCfg::DianKongFangFen_LowerLimit)
	{
		if (llFangFen >= CFishServerCfg::DianKongFangFen_UpperLimit)
		{
			//设置放分上限，防止单次放分过猛.
			llFangFen = CFishServerCfg::DianKongFangFen_UpperLimit;
		}
		LOG(INFO) << "系统点控输赢总分为: " << m_llSysDianKongWinLoseTotalScore << " 实际放分: " << llFangFen;

		std::vector<CT_LONGLONG> vecFen;
		if (rand() % 2 == 0)
		{
			//10% 20% %20 %20 30%
			std::vector<CT_WORD> vecPro;
			vecPro.push_back(10);
			vecPro.push_back(20);
			vecPro.push_back(20);
			vecPro.push_back(20);
			vecPro.push_back(30);
			std::random_shuffle(vecPro.begin(), vecPro.end());

			CT_DOUBLE dPro, dTempFen;
			CT_LONGLONG llTempFen;
			for (size_t i = 0; i < vecPro.size(); i++)
			{
				dPro = vecPro[i];
				dTempFen = (dPro / 100.0f) * llFangFen;
				llTempFen = dTempFen;
				vecFen.push_back(llTempFen);
			}
		}
		else
		{
			// 10% 20% 30% 40% 
			std::vector<CT_WORD> vecPro;
			vecPro.push_back(10);
			vecPro.push_back(20);
			vecPro.push_back(30);
			vecPro.push_back(40);
			std::random_shuffle(vecPro.begin(), vecPro.end());

			CT_DOUBLE dPro, dTempFen;
			CT_LONGLONG llTempFen;
			for (size_t i = 0; i < vecPro.size(); i++)
			{
				dPro = vecPro[i];
				dTempFen = (dPro / 100.0f) * llFangFen;
				llTempFen = dTempFen;
				vecFen.push_back(llTempFen);
			}
		}

		for (size_t i = 0; i < m_vecUserAddScore.size() && i < vecFen.size(); i++)
		{
			if (m_vecUserAddScore[i].llAddScore < 0)
			{
				if (m_vecUserAddScore[i].pTableFrameSink)
				{
					MSG_D2CS_Set_FishDiankong data;
					//if (rand() % 2 == 0)
					//{
					//暂时使用最大的力度，否则点控持续时间较长
						data.bySource = 3;
						data.dwUserID = m_vecUserAddScore[i].dwUserID;
						data.iDianKongZhi = -30;
						data.llDianKongFen = vecFen[i];
						data.nBaseCapPro = 11990;
						data.nBloodPoolStateValue = 50000;
					//}
					//else
					//{
					//	data.bySource = 3;
					//	data.dwUserID = m_vecUserAddScore[i].dwUserID;
					//	data.iDianKongZhi = -10;
					//	data.llDianKongFen = vecFen[i];
					//	data.nBaseCapPro = 10990;
					//	data.nBloodPoolStateValue = 25000;
					//}
					if (m_vecUserAddScore[i].pTableFrameSink->SetFishDianKongData(&data))
					{
						LOG(INFO) << "系统点控玩家: " << data.dwUserID << " 赢" << data.llDianKongFen << " 分" << " 玩家当前输了 " << m_vecUserAddScore[i].llAddScore << " 分";
						m_vecCurrentDianKongWinUsers.push_back(data.dwUserID);
						m_bStartAddScore = CT_TRUE;
					}
					else
					{
						continue;
					}
				}
			}
		}
	}
}

CT_VOID CFishControl::SaveFishTodayTotalWinLoseScore()
{
	MSG_GS2DB_FishTotalWinLoseScore data;
	data.llTotalLoseScore = m_llTodayTotalLoseScore;
	data.llTotalWinScore = m_llTodayTotalWinScore;
	data.llTodayTotalKuCun = m_llTodayTotalKuCun;
	data.llTodayTotalTax = m_llTodayTotalTax;
	data.llTodayDianKongWinLoseTotalScore = m_llTodayDianKongWinLoseTotalScore;
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		data.llTotalLoseScore = data.llTotalLoseScore / 10;
		data.llTotalWinScore = data.llTotalWinScore / 10;
		data.llTodayTotalKuCun = data.llTodayTotalKuCun / 10;
		data.llTodayTotalTax = data.llTodayTotalTax / 10;
		data.llTodayDianKongWinLoseTotalScore = data.llTodayDianKongWinLoseTotalScore / 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		data.llTotalLoseScore = data.llTotalLoseScore * 10;
		data.llTotalWinScore = data.llTotalWinScore * 10;
		data.llTodayTotalKuCun = data.llTodayTotalKuCun * 10;
		data.llTodayTotalTax = data.llTodayTotalTax * 10;
		data.llTodayDianKongWinLoseTotalScore = data.llTodayDianKongWinLoseTotalScore * 10;
	}
	data.serverID = CServerCfg::m_nServerID;
	tm times;
	Utility::getLocalTime(&times, m_nTodayTime);
	data.nYear = times.tm_year + 1900;
	data.nMonth = times.tm_mon + 1;
	data.nDay = times.tm_mday;

	data.dwTotalEnterCount = m_dwTodayTotalEnterCount;
	data.dwTotalWinCount = m_dwTodayTotalPlayerCount_Win;
	data.dwTotalPoChanCount = m_dwTodayTotalPlayerCount_Bankrupted;
	data.dwTotalRewardCount = m_dwTodayRewardCount;
	data.dwTotalXiFenCount = m_dwTodayXiFenCount;
	if(m_mapTodayBigFishInfo.size() <= TONG_JI_BIG_FISH_COUNT)
	{
		int nIndex = 0;
		for(std::map<CT_BYTE, sBigFishInFo>::iterator it = m_mapTodayBigFishInfo.begin(); it != m_mapTodayBigFishInfo.end(); it++)
		{
			data.bfInfo[nIndex].cbFishType = it->first;
			data.bfInfo[nIndex].bfInfo.llPlayerGetScore = it->second.llPlayerGetScore;
			data.bfInfo[nIndex].bfInfo.llPlayerLostScore = it->second.llPlayerLostScore;
			nIndex++;
			if(nIndex >= TONG_JI_BIG_FISH_COUNT)
			{
				break;
			}
		}
		data.cbbfInfoCount = nIndex;
	}
	if(m_mapTodayProInfo.size() <= BASE_PRO_MAX_COUNT)
	{
		int nIndex = 0;
		for(std::map<CT_DWORD, CT_DWORD>::iterator it = m_mapTodayProInfo.begin(); it != m_mapTodayProInfo.end(); it++)
		{
			data.proInfo[nIndex].dwBasePro = it->first;
			data.proInfo[nIndex].dwCheckCount = it->second;
			nIndex++;
			if(nIndex >= BASE_PRO_MAX_COUNT)
			{
				break;
			}
		}
		data.cbproInfoCount = nIndex;
	}
	else
	{
	    LOG(ERROR) << "ServerID:"<<data.serverID << "m_mapTodayProInfo.size():"<<m_mapTodayProInfo.size() << "data.cbproInfoCount:" << data.cbproInfoCount;
	}

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE, &data, sizeof(MSG_GS2DB_FishTotalWinLoseScore));
	//LOG(ERROR) << "发送SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE";
	m_llTodayTotalLoseScore = 0;
	m_llTodayTotalWinScore = 0;
	m_llTodayTotalKuCun = 0;
	m_llTodayTotalTax = 0; 
	m_llTodayDianKongWinLoseTotalScore = 0;
	m_nTodayTime = time(NULL);
}

CT_VOID CFishControl::Init()
{
	m_llTestZhengTotal = 0;
	m_llTestZhengCnt = 0;
	m_llTestFuTotal = 0;
	m_llTestFuCnt = 0;

	m_nTestBeginTime = time(NULL);
	 m_dwPlayerCnt = 0;
	 m_llWinMost = 0;
	 m_llLoseMost = 0;
	 m_dwPoChanPlayerCnt = 0;
	 m_dwWinPlayerCnt = 0;

	 m_nTestDengYu1650Cnt = 0;
	 m_nTestDaYu1650Cnt = 0;

	 m_llKuCunZhengShuCnt = 0;
	 m_llKuCunFuShuCnt = 0;

	 m_llTestBaseCapProDaYu10000 = 0;
	 m_llTestBaseCapProXiaoYu10000 = 0;
	//只有临时变量在每次服务器重启的时候才重置为0，其他的几个控制数据都从数据库读取
	m_nTaxTempValue = 0;

	m_llTodayTotalLoseScore = 0;
	m_llTodayTotalWinScore = 0;
	m_llTodayTotalKuCun = 0;
	m_llTodayTotalTax = 0;
	m_llTodayDianKongWinLoseTotalScore = 0; 
	m_nTodayTime = time(NULL);

	pFishSerCfg->LoadFishBloodPoolStateCfg();
	CheckBaseCapturePro();

	m_dTaxPro = CFishServerCfg::TaxPro;
	m_dTaxPro = m_dTaxPro / 10000;

	m_llSysDianKongWinTotalScore = 0;
	m_llSysDianKongLoseTotalScore = 0;
	m_llSysDianKongWinLoseTotalScore = 0;
	m_bStartAddScore = CT_FALSE;

	for (int i = 0; i < 40; i++)
	{
		m_FishStatisticsInfo[i].ResetData();
	}
	//int nBaseCapPro = 0;
	//CGameTableManager::get_instance().InitAllTableBaseCapPro(nBaseCapPro);

	m_dwTodayTotalEnterCount = 0;
	m_dwTodayTotalPlayerCount_Win = 0;
	m_dwTodayTotalPlayerCount_Bankrupted = 0;
	m_dwTodayTotalPlayerCount_Lose = 0;
	m_dwTodayTatalPlayerCount_NoWinLose = 0;
	m_dwTodayForbidPoChanReward = 0;
	m_mapTodayBigFishInfo.clear();
	m_dwTodayRewardCount = 0;
	m_dwTodayXiFenCount = 0;
	m_mapTodayProInfo.clear();
}

CT_VOID CFishControl::NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llSysWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose)
{
	CT_LONGLONG llOriginal = llBloodPoolDianKongWinLose;
	//不同的场次有不同的汇率
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llSysWinScore *= 10;
		llKuCun *= 10;
		llBloodPoolDianKongWinLose *= 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		CT_DOUBLE dSysWinScore = llSysWinScore;
		dSysWinScore = dSysWinScore * 0.1;
		dSysWinScore = std::round(dSysWinScore);
		llSysWinScore = dSysWinScore;

		CT_DOUBLE dKuCun = llKuCun;
		dKuCun = dKuCun * 0.1;
		dKuCun = std::round(dKuCun);
		llKuCun = dKuCun;

		CT_DOUBLE dBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose;
		dBloodPoolDianKongWinLose = dBloodPoolDianKongWinLose * 0.1;
		dBloodPoolDianKongWinLose = std::round(dBloodPoolDianKongWinLose);
		llBloodPoolDianKongWinLose = dBloodPoolDianKongWinLose;
	}

	m_llTodayTotalWinScore += llSysWinScore;
	m_nAllTableTotalWinScore += llSysWinScore;
	m_nTaxTempValue += llSysWinScore;
	m_nAllTableKuCun += llKuCun;
	m_llTodayTotalKuCun += llKuCun;
	m_llBloodPoolDianKongWinLose += llBloodPoolDianKongWinLose;
	m_llTodayDianKongWinLoseTotalScore += llBloodPoolDianKongWinLose;
	m_mapPlayerDianKongWinLose[dwUserID] -= llOriginal;

	//CT_DOUBLE dTaxValue = 0;
	if (m_nTaxTempValue >= 10000)
	{
		//本次从吸分中抽税
		CT_LONGLONG llTemp = ((m_nTaxTempValue / 10000) * 10000);
		CT_INT64 nTaxValue = (llTemp * CFishServerCfg::TaxPro) / 10000;
		//从库存中减去本次的税收
		m_nAllTableKuCun -= nTaxValue;
		m_llTodayTotalKuCun -= nTaxValue;
		//统计到总税收
		m_nAllTableTotalTax += nTaxValue;
		m_llTodayTotalTax += nTaxValue;
        //对临时变量取余修正
        m_nTaxTempValue = m_nTaxTempValue % 10000;
		//WrtLog(1, "【状态:吸分时抽税】税收临时值: %d ,本次收税:%d ,税收临时值修正后:%d ,目前累计总税收:%d", nPreTaxTempValue,
		//nTaxValue, m_nTaxTempValue, m_nAllTableTotalTax);
		//LOG(ERROR) << "【状态:吸分】税收临时值: " << nPreTaxTempValue << " 本次收税: " << nTaxValue << " 税收临时值修正后: " << m_nTaxTempValue << " 目前累计总税收: " << m_nAllTableTotalTax;
	}

	if (m_nAllTableKuCun > 0)
	{
		m_llKuCunZhengShuCnt++;
	}
	else if (m_nAllTableKuCun < 0)
	{
		m_llKuCunFuShuCnt++;
	}

	CT_LONGLONG llKuCunBoDong = std::abs(m_nAllTableKuCun) - std::abs(m_llKuCunCaiYang);
	if (llKuCunBoDong >= Hight_Point || llKuCunBoDong <= Low_Point)
	{
		sKuCunCaiYangInfo cyInfo;
		cyInfo.nCaiYangTime = time(NULL);
		cyInfo.llKuCuCaiYang = m_nAllTableKuCun;
		if (CFishServerCfg::IsSaveCurrentKuCun == 1)
		{
			//配置中允许保存实时库存才把数据加入,否则内存一直会增加
			m_listKuCunCaiYang.push_back(cyInfo);
		}
		m_llKuCunCaiYang = m_nAllTableKuCun;
		//关闭日志
		//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);

		//关闭日志
		//CT_INT32 nTestBaseCapPro = GetBaseCapturPro(m_nAllTableKuCun);
		//WrtLog(CServerCfg::m_nServerID * 10, 1, "%d", nTestBaseCapPro);

		//if (nTestBaseCapPro > 10000)
		//{
		//	m_llTestBaseCapProDaYu10000++;
		//}
		//if (nTestBaseCapPro < 10000)
		//{
		//	m_llTestBaseCapProXiaoYu10000++;
		//}

		if (m_nAllTableKuCun > m_llKuCunMostTop)
		{
			m_llKuCunMostTop = m_nAllTableKuCun;
		}
		if (m_nAllTableKuCun < m_llKuCunMostLow)
		{
			m_llKuCunMostLow = m_nAllTableKuCun;
		}
	}

	//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);
	//LOG(INFO) << m_nAllTableKuCun << " ";
	//WrtLog(1, "%d", m_nAllTableKuCun);
	//WrtLog(1, "总吸分:%lld, 总税收:%lld, 库存:%lld", m_nAllTableTotalWinScore, m_nAllTableTotalTax, m_nAllTableKuCun);
}

//玩家点控开始
CT_VOID CFishControl::UserDianKongStart(CT_DWORD dwUserID)
{
	m_mapPlayerDianKongWinLose[dwUserID] = 0;

	//LOG(INFO) << "玩家ID: " << dwUserID << " 开始点控";
}

//玩家点控结束
CT_VOID CFishControl::UserDianKongEnd(CT_DWORD dwUserID)
{
	CT_LONGLONG llUserDianKongWinLose = m_mapPlayerDianKongWinLose[dwUserID];
	if (llUserDianKongWinLose > 0)
	{
		m_llSysDianKongLoseTotalScore = m_llSysDianKongLoseTotalScore + llUserDianKongWinLose;
		m_llSysDianKongWinLoseTotalScore = m_llSysDianKongWinLoseTotalScore - llUserDianKongWinLose;
	}
	else
	{
		m_llSysDianKongWinTotalScore = m_llSysDianKongWinTotalScore + (llUserDianKongWinLose * (-1));
		m_llSysDianKongWinLoseTotalScore = m_llSysDianKongWinLoseTotalScore + (llUserDianKongWinLose * (-1));
	}

	for (std::vector<CT_DWORD>::iterator it = m_vecCurrentDianKongWinUsers.begin(); it != m_vecCurrentDianKongWinUsers.end();)
	{
		if (*it == dwUserID)
		{
			it = m_vecCurrentDianKongWinUsers.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (m_vecCurrentDianKongWinUsers.size() == 0)
	{
		//这一波点控放分已经结束,可以开始下一波.
		m_bStartAddScore = CT_FALSE;
	}

	m_mapPlayerDianKongWinLose[dwUserID] = 0;
	//LOG(INFO) << "玩家ID: " << dwUserID << " 结束点控,点控输赢值为: " << llUserDianKongWinLose << "目前系统点控输赢总分为: " << m_llSysDianKongWinLoseTotalScore;
}

//玩家每发一炮就通知系统赢了多少分fv
CT_VOID CFishControl::NotifySysWinScore(CT_LONGLONG llSysWinScore)
{
	//不同的场次有不同的汇率
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llSysWinScore *= 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		CT_DOUBLE dSysWinScore = llSysWinScore;
		dSysWinScore = dSysWinScore * 0.1;
		dSysWinScore = std::round(dSysWinScore);
		llSysWinScore = dSysWinScore;
	}

	m_llTodayTotalWinScore += llSysWinScore;
	m_nAllTableTotalWinScore += llSysWinScore;
	m_nAllTableKuCun += llSysWinScore;
	m_llTodayTotalKuCun += llSysWinScore;
	m_nTaxTempValue += llSysWinScore;

	//CT_DOUBLE dTaxValue = 0;
	if (m_nTaxTempValue >= 10000)
	{
		//本次从吸分中抽税
		CT_LONGLONG llTemp = ((m_nTaxTempValue / 10000) * 10000);
		CT_INT64 nTaxValue = (llTemp * CFishServerCfg::TaxPro) / 10000;
		//从库存中减去本次的税收
		m_nAllTableKuCun -= nTaxValue;
		m_llTodayTotalKuCun -= nTaxValue;
		//统计到总税收
		m_nAllTableTotalTax += nTaxValue;
		m_llTodayTotalTax += nTaxValue;
        //对临时变量取余修正
        m_nTaxTempValue = m_nTaxTempValue % 10000;
		//WrtLog(1, "【状态:吸分时抽税】税收临时值: %d ,本次收税:%d ,税收临时值修正后:%d ,目前累计总税收:%d", nPreTaxTempValue,
			//nTaxValue, m_nTaxTempValue, m_nAllTableTotalTax);
		//LOG(ERROR) << "【状态:吸分】税收临时值: " << nPreTaxTempValue << " 本次收税: " << nTaxValue << " 税收临时值修正后: " << m_nTaxTempValue << " 目前累计总税收: " << m_nAllTableTotalTax;
	}


	if (m_nAllTableKuCun > 0)
	{
		m_llKuCunZhengShuCnt++;
	}
	else if (m_nAllTableKuCun < 0)
	{
		m_llKuCunFuShuCnt++;
	}

	CT_LONGLONG llKuCunBoDong = std::abs(m_nAllTableKuCun) - std::abs(m_llKuCunCaiYang);
	if (llKuCunBoDong >= Hight_Point || llKuCunBoDong <= Low_Point)
	{
		sKuCunCaiYangInfo cyInfo;
		cyInfo.nCaiYangTime = time(NULL);
		cyInfo.llKuCuCaiYang = m_nAllTableKuCun;
		if (CFishServerCfg::IsSaveCurrentKuCun == 1)
		{
			//配置中允许保存实时库存才把数据加入,否则内存一直会增加
			m_listKuCunCaiYang.push_back(cyInfo);
		}
		m_llKuCunCaiYang = m_nAllTableKuCun;

		//关闭日志
		//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);

		//关闭日志
		//CT_INT32 nTestBaseCapPro = GetBaseCapturPro(m_nAllTableKuCun);
		//WrtLog(CServerCfg::m_nServerID * 10, 1, "%d", nTestBaseCapPro);

		//if (nTestBaseCapPro > 10000)
		//{
		//	m_llTestBaseCapProDaYu10000++;
		//}
		//if (nTestBaseCapPro < 10000)
		//{
		//	m_llTestBaseCapProXiaoYu10000++;
		//}

		if (m_nAllTableKuCun > m_llKuCunMostTop)
		{
			m_llKuCunMostTop = m_nAllTableKuCun;
		}
		if (m_nAllTableKuCun < m_llKuCunMostLow)
		{
			m_llKuCunMostLow = m_nAllTableKuCun;
		}
	}
	//WrtLog(CServerCfg::m_nServerID ,1, "%lld", m_nAllTableKuCun);
	//WrtLog(1, "【状态:吸分】本次吸分: %d 累计总吸分: %d 库存:%d", llSysWinScore,m_nAllTableTotalWinScore, m_nAllTableKuCun);
	//LOG(ERROR) << "【状态:吸分】总吸分: " << m_nAllTableTotalWinScore << " 库存: " << m_nAllTableKuCun;
}

CT_VOID CFishControl::NotifySysLoseScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llSysLoseScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose)
{
	CT_LONGLONG llOriginal = llBloodPoolDianKongWinLose;
	//不同的场次有不同的汇率
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llSysLoseScore *= 10;
		llKuCun *= 10;
		llBloodPoolDianKongWinLose *= 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		CT_DOUBLE dSysLoseScore = llSysLoseScore;
		dSysLoseScore = dSysLoseScore * 0.1;
		dSysLoseScore = std::round(dSysLoseScore);
		llSysLoseScore = dSysLoseScore;

		CT_DOUBLE dKuCun = llKuCun;
		dKuCun = dKuCun * 0.1;
		dKuCun = std::round(dKuCun);
		llKuCun = dKuCun;

		CT_DOUBLE dBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose;
		dBloodPoolDianKongWinLose = dBloodPoolDianKongWinLose * 0.1;
		dBloodPoolDianKongWinLose = std::round(dBloodPoolDianKongWinLose);
		llBloodPoolDianKongWinLose = dBloodPoolDianKongWinLose;
	}

	m_llTodayTotalLoseScore += llSysLoseScore;
	m_nAllTableTotalLoseScore += llSysLoseScore;
	m_nTaxTempValue += llSysLoseScore;
	m_nAllTableKuCun -= llKuCun;
	m_llTodayTotalKuCun -= llKuCun;
	m_llBloodPoolDianKongWinLose -= llBloodPoolDianKongWinLose;
	m_llTodayDianKongWinLoseTotalScore -= llBloodPoolDianKongWinLose;
	m_mapPlayerDianKongWinLose[dwUserID] += llOriginal;

	//CT_DOUBLE dTaxValue = 0;
	if (m_nTaxTempValue >= 10000)
	{
        //本次从吸分中抽税
        CT_LONGLONG llTemp = ((m_nTaxTempValue / 10000) * 10000);
        CT_INT64 nTaxValue = (llTemp * CFishServerCfg::TaxPro) / 10000;
        //从库存中减去本次的税收
        m_nAllTableKuCun -= nTaxValue;
        m_llTodayTotalKuCun -= nTaxValue;
        //统计到总税收
        m_nAllTableTotalTax += nTaxValue;
        m_llTodayTotalTax += nTaxValue;
        //对临时变量取余修正
        m_nTaxTempValue = m_nTaxTempValue % 10000;
		//WrtLog(1, "【状态:放分时抽税】税收临时值: %d ,本次收税:%d ,税收临时值修正后:%d ,目前累计总税收:%d", nPreTaxTempValue,
		//nTaxValue, m_nTaxTempValue, m_nAllTableTotalTax);
		//LOG(ERROR) << "【状态:放分】税收临时值: " << nPreTaxTempValue << " 本次收税: " << nTaxValue << " 税收临时值修正后: " << m_nTaxTempValue << " 目前累计总税收: " << m_nAllTableTotalTax;
	}

	if (m_nAllTableKuCun > 0)
	{
		m_llKuCunZhengShuCnt++;
	}
	else if (m_nAllTableKuCun < 0)
	{
		m_llKuCunFuShuCnt++;
	}

	CT_LONGLONG llKuCunBoDong = std::abs(m_nAllTableKuCun) - std::abs(m_llKuCunCaiYang);
	if (llKuCunBoDong >= Hight_Point || llKuCunBoDong <= Low_Point)
	{
		sKuCunCaiYangInfo cyInfo;
		cyInfo.nCaiYangTime = time(NULL);
		cyInfo.llKuCuCaiYang = m_nAllTableKuCun;
		if (CFishServerCfg::IsSaveCurrentKuCun == 1)
		{
			//配置中允许保存实时库存才把数据加入,否则内存一直会增加
			m_listKuCunCaiYang.push_back(cyInfo);
		}
		m_llKuCunCaiYang = m_nAllTableKuCun;

		//关闭日志
		//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);

		//关闭日志
		//CT_INT32 nTestBaseCapPro = GetBaseCapturPro(m_nAllTableKuCun);
		//WrtLog(CServerCfg::m_nServerID * 10, 1, "%d", nTestBaseCapPro);

		//if (nTestBaseCapPro > 10000)
		//{
		//	m_llTestBaseCapProDaYu10000++;
		//}
		//if (nTestBaseCapPro < 10000)
		//{
		//	m_llTestBaseCapProXiaoYu10000++;
		//}

		if (m_nAllTableKuCun > m_llKuCunMostTop)
		{
			m_llKuCunMostTop = m_nAllTableKuCun;
		}
		if (m_nAllTableKuCun < m_llKuCunMostLow)
		{
			m_llKuCunMostLow = m_nAllTableKuCun;
		}
	}
	//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);
	//LOG(INFO) << m_nAllTableKuCun << " ";
	//WrtLog(1, "%d", m_nAllTableKuCun);
	//WrtLog(1, "总放分:%lld, 总税收:%lld, 库存:%lld", m_nAllTableTotalLoseScore, m_nAllTableTotalTax, m_nAllTableKuCun);
}

//玩家每次得分就通知系统输了多少分
CT_VOID CFishControl::NotifySysLoseScore(CT_LONGLONG llSysLoseScore)
{
	//不同的场次有不同的汇率
	if (CFishServerCfg::m_exchangeRate == 1)
	{
		
	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llSysLoseScore *= 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		CT_DOUBLE dSysLoseScore = llSysLoseScore;
		dSysLoseScore = dSysLoseScore * 0.1;
		dSysLoseScore = std::round(dSysLoseScore);
		llSysLoseScore = dSysLoseScore;
	}

	m_llTodayTotalLoseScore += llSysLoseScore;
	m_nAllTableTotalLoseScore += llSysLoseScore;
	m_nAllTableKuCun -= llSysLoseScore;
	m_llTodayTotalKuCun -= llSysLoseScore;
	m_nTaxTempValue += llSysLoseScore;

	//CT_DOUBLE dTaxValue = 0;
	if (m_nTaxTempValue >= 10000)
	{
        //本次从吸分中抽税
        CT_LONGLONG llTemp = ((m_nTaxTempValue / 10000) * 10000);
        CT_INT64 nTaxValue = (llTemp * CFishServerCfg::TaxPro) / 10000;
        //从库存中减去本次的税收
        m_nAllTableKuCun -= nTaxValue;
        m_llTodayTotalKuCun -= nTaxValue;
        //统计到总税收
        m_nAllTableTotalTax += nTaxValue;
        m_llTodayTotalTax += nTaxValue;
        //对临时变量取余修正
        m_nTaxTempValue = m_nTaxTempValue % 10000;
		//WrtLog(1, "【状态:放分时抽税】税收临时值: %d ,本次收税:%d ,税收临时值修正后:%d ,目前累计总税收:%d", nPreTaxTempValue,
			//nTaxValue, m_nTaxTempValue, m_nAllTableTotalTax);
		//LOG(ERROR) << "【状态:放分】税收临时值: " << nPreTaxTempValue << " 本次收税: " << nTaxValue << " 税收临时值修正后: " << m_nTaxTempValue << " 目前累计总税收: " << m_nAllTableTotalTax;
	}

	if (m_nAllTableKuCun > 0)
	{
		m_llKuCunZhengShuCnt++;
	}
	else if (m_nAllTableKuCun < 0)
	{
		m_llKuCunFuShuCnt++;
	}


	CT_LONGLONG llKuCunBoDong = std::abs(m_nAllTableKuCun) - std::abs(m_llKuCunCaiYang);
	if (llKuCunBoDong >= Hight_Point || llKuCunBoDong <= Low_Point)
	{
		sKuCunCaiYangInfo cyInfo;
		cyInfo.nCaiYangTime = time(NULL);
		cyInfo.llKuCuCaiYang = m_nAllTableKuCun;
		if (CFishServerCfg::IsSaveCurrentKuCun == 1)
		{
			//配置中允许保存实时库存才把数据加入,否则内存一直会增加
			m_listKuCunCaiYang.push_back(cyInfo);
		}
		m_llKuCunCaiYang = m_nAllTableKuCun;

		//关闭日志
		//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);

		//关闭日志
		//CT_INT32 nTestBaseCapPro = GetBaseCapturPro(m_nAllTableKuCun);
		//WrtLog(CServerCfg::m_nServerID * 10, 1, "%d", nTestBaseCapPro);

		//if (nTestBaseCapPro > 10000)
		//{
		//	m_llTestBaseCapProDaYu10000++;
		//}
		//if (nTestBaseCapPro < 10000)
		//{
		//	m_llTestBaseCapProXiaoYu10000++;
		//}

		if (m_nAllTableKuCun > m_llKuCunMostTop)
		{
			m_llKuCunMostTop = m_nAllTableKuCun;
		}
		if (m_nAllTableKuCun < m_llKuCunMostLow)
		{
			m_llKuCunMostLow = m_nAllTableKuCun;
		}
	}
	//WrtLog(CServerCfg::m_nServerID,1, "%lld", m_nAllTableKuCun);
	//WrtLog(1, "【状态:放分】本次放分: %d 累计总放分: %d 库存:%d", llSysLoseScore, m_nAllTableTotalLoseScore, m_nAllTableKuCun);
	//LOG(ERROR) << "【状态:放分】总放分 " << m_nAllTableTotalLoseScore << " 库存: " << m_nAllTableKuCun;
}

CT_VOID CFishControl::PrintFishControlData()
{
	CT_LONGLONG llTotalTax = m_nAllTableTotalTax;
	CT_LONGLONG llTotalKuCun = m_nAllTableKuCun;
	CT_LONGLONG llTotalLoseScore = m_nAllTableTotalLoseScore;
	CT_LONGLONG llTotalWinScore = m_nAllTableTotalWinScore;
	CT_LONGLONG llBloodPoolDianKongWinLose = m_llBloodPoolDianKongWinLose;
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llTotalTax = llTotalTax / 10;
		llTotalKuCun = llTotalKuCun / 10;
		llTotalLoseScore = llTotalLoseScore / 10;
		llTotalWinScore = llTotalWinScore / 10;
		llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose / 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		llTotalTax = llTotalTax * 10;
		llTotalKuCun = llTotalKuCun * 10;
		llTotalLoseScore = llTotalLoseScore * 10;
		llTotalWinScore = llTotalWinScore * 10;
		llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose * 10;
	}

	CT_DOUBLE dTotalEnterCount = m_dwTodayTotalEnterCount;
	CT_DOUBLE dTotalWinCount = m_dwTodayTotalPlayerCount_Win;
	CT_DOUBLE dTotalPoChanCount = m_dwTodayTotalPlayerCount_Bankrupted;
	CT_DOUBLE dTotalLoseCount = m_dwTodayTotalPlayerCount_Lose;
	CT_DOUBLE dTotalNoWinLose = m_dwTodayTatalPlayerCount_NoWinLose;
	CT_DOUBLE dWinPro = dTotalWinCount / dTotalEnterCount;
	CT_DOUBLE dPoChanPro = dTotalPoChanCount / dTotalEnterCount;
	CT_DOUBLE dLosePro = dTotalLoseCount / dTotalEnterCount;
	CT_DOUBLE dNoWinLosePro = dTotalNoWinLose / dTotalEnterCount;
	/*LOG(INFO) << "服务器ID: " << CServerCfg::m_nServerID << " 总税收: " << llTotalTax << " 总库存: " << llTotalKuCun
		<< " 总放分: " << llTotalLoseScore << " 总吸分: " << llTotalWinScore << "点控输赢: " << llBloodPoolDianKongWinLose
		<< " 库存最高点: " << m_llKuCunMostTop << " 库存最低点: " << m_llKuCunMostLow << " 库存正数部分: " << m_llKuCunZhengShuCnt
		<< " 库存负数部分: " << m_llKuCunFuShuCnt << " 基础捕获概率大于10000数量: " << m_llTestBaseCapProDaYu10000 
		<< " 基础捕获概率小于10000数量: " << m_llTestBaseCapProXiaoYu10000 << "胜率:" << dWinPro << " 破产率: "<< dPoChanPro;*/
	LOG(INFO) << "胜率:" << dWinPro << " 破产率: "<< dPoChanPro << " 输率:" << dLosePro << " 不输不赢:" << dNoWinLosePro
	<< " 总进入数:" << m_dwTodayTotalEnterCount << " 总赢次数: "<< m_dwTodayTotalPlayerCount_Win
	<<" 总破产次数:"<< m_dwTodayTotalPlayerCount_Bankrupted << " 总输次数:" << m_dwTodayTotalPlayerCount_Lose
	<< " 总不输不赢次数:"<< m_dwTodayTatalPlayerCount_NoWinLose << " 防止破产奖励:" << m_dwTodayForbidPoChanReward;

	int nTotalTimes = 0;
    for (std::map<int, CT_UINT32>::iterator itTest = m_mapTestCapProCnt.begin(); itTest != m_mapTestCapProCnt.end(); itTest++)
    {
        nTotalTimes += itTest->second;
    }

	int nZCnt = 0, nFCnt = 0;
	for (std::map<int, CT_UINT32>::iterator itTest = m_mapTestCapProCnt.begin(); itTest != m_mapTestCapProCnt.end(); itTest++)
	{
        CT_DOUBLE  fff = (CT_DOUBLE)itTest->second / (CT_DOUBLE)nTotalTimes * 100.0f;
	    CT_DOUBLE  ddd;
	    if(itTest->first > 10000)
        {
	        ddd = (CT_DOUBLE)((itTest->first - 10000)) / 10000.0f * 100.0f;
            LOG(INFO) << "基础捕获概率: " << itTest->first << " 被设置的次数: " << itTest->second << "      " << "+"<<ddd << "%" <<"     " << fff <<"%";
        }
        else if(itTest->first < 10000)
        {
            ddd = (CT_DOUBLE)(10000 - itTest->first) / 10000.0f * 100.0f;
            LOG(INFO) << "基础捕获概率: " << itTest->first << " 被设置的次数: " << itTest->second << "      " << "-"<<ddd << "%" <<"     " << fff <<"%";
        }

		if (itTest->first > 10000)
		{
			nZCnt += itTest->second;
		}
		if (itTest->first < 10000)
		{
			nFCnt += itTest->second;
		}
	}

	LOG(INFO) << "系统放分次数: " << nZCnt << " 系统吸分次数: " << nFCnt;

	CT_DOUBLE dTempTimes = (time(NULL) - m_nTestBeginTime);
	CT_DOUBLE dTimers = dTempTimes / 60;

	CT_DOUBLE dPlayerCnt = m_dwPlayerCnt;
	CT_DOUBLE dPoChanCnt = m_dwPoChanPlayerCnt;
	CT_DOUBLE dWinPlayerCnt = m_dwWinPlayerCnt;

	//计算输赢最多的值
	CT_LONGLONG llWinMost = 0, llLoseMost = 0;
	for (std::map<CT_DWORD, CT_LONGLONG>::iterator itWinLostMost = m_mapTestPlayerMostWinLoseScore.begin(); itWinLostMost != m_mapTestPlayerMostWinLoseScore.end(); itWinLostMost++)
	{
		if (itWinLostMost->second > llWinMost)
		{
			llWinMost = itWinLostMost->second;
		}
		if (itWinLostMost->second < llLoseMost)
		{
			llLoseMost = itWinLostMost->second;
		}
	}

	CT_DOUBLE dPoChanLv = 0;
	CT_DOUBLE dWinLv = 0;
	if(dPlayerCnt != 0)
		dPoChanLv = dPoChanCnt / dPlayerCnt;
	if(dPlayerCnt != 0)
		dWinLv = dWinPlayerCnt / dPlayerCnt;
	LOG(INFO) << "总人数: " << m_dwPlayerCnt << " 总时长: " << dTimers << "分钟" << " 赢得最多: " << llWinMost << " 输得最多: "
		<< llLoseMost << " 破产率: " << dPoChanLv << " 赢家比例: " << dWinLv
		<<" 赢家人数: "<< m_dwWinPlayerCnt << " 破产人数: "<< m_dwPoChanPlayerCnt;

	LOG(INFO) << "所有玩家总税收:" << m_llTestAllPlayerTotalTax;

/*	CT_INT32 llTestZhengPingJun = m_llTestZhengTotal / m_llTestZhengCnt;
	CT_INT32 llTestFuPingJun = (m_llTestFuTotal * (-1)) / m_llTestFuCnt;
	CT_INT32 llTestJianYiPingJun = 0;
	if (llTestFuPingJun > llTestZhengPingJun)
	{
		llTestJianYiPingJun = 0 - (llTestFuPingJun - llTestZhengPingJun) / 2;
	}
	else
	{
		llTestJianYiPingJun = (llTestZhengPingJun - llTestFuPingJun) / 2;
	}

	CT_INT32 nTestFuJun = llTestFuPingJun*(-1);
	LOG(INFO) << "正数部分平均值:" << llTestZhengPingJun << " 负数部分平均值:"<< nTestFuJun << " 建议平均线: " << llTestJianYiPingJun;

	CT_INT32 nn = (m_llTestZhengTotal + m_llTestFuTotal) / (m_llTestZhengCnt + m_llTestFuCnt);
	LOG(INFO) << "建议平均线2:" << nn;*/
}

CT_VOID CFishControl::SaveFishControlData(acl::mysql_manager &dbManager)
{
	/*
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(ERROR) << "SaveFishControlData get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(ERROR) << "SaveFishControlData get platform db handle fail.";
		return;
	}
	*/

	CT_LONGLONG llTotalTax = m_nAllTableTotalTax;
	CT_LONGLONG llTotalKuCun = m_nAllTableKuCun;
	CT_LONGLONG llTotalLoseScore = m_nAllTableTotalLoseScore;
	CT_LONGLONG llTotalWinScore = m_nAllTableTotalWinScore;
	CT_LONGLONG llBloodPoolDianKongWinLose = m_llBloodPoolDianKongWinLose;
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llTotalTax = llTotalTax / 10;
		llTotalKuCun = llTotalKuCun / 10;
		llTotalLoseScore = llTotalLoseScore / 10;
		llTotalWinScore = llTotalWinScore / 10;
		llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose / 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		llTotalTax = llTotalTax * 10;
		llTotalKuCun = llTotalKuCun * 10;
		llTotalLoseScore = llTotalLoseScore * 10;
		llTotalWinScore = llTotalWinScore * 10;
		llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose * 10;
	}

	//LOG(ERROR) << "【调试答应】总吸分: " << llTotalWinScore << " 总放分: " << llTotalLoseScore << " 暗税: " << llTotalTax << " 库存: "
		//<< llTotalKuCun << " 血池点控输赢: " << llBloodPoolDianKongWinLose;
	/*
	acl::query query;
	query.create_sql("INSERT INTO FishControlData(ServerID,TotalWinScore,TotalLoseScore,KuCun,BlackTax,BloodPoolDianKongLoseWin) VALUES (:ServerID, :TotalWinScore, :TotalLoseScore, :KuCun, :BlackTax, :BloodPoolDianKongLoseWin) \
    ON DUPLICATE KEY UPDATE TotalWinScore=:TotalWinScore, TotalLoseScore=:TotalLoseScore, KuCun=:KuCun,BlackTax=:BlackTax,BloodPoolDianKongLoseWin=:BloodPoolDianKongLoseWin")
		.set_format("BlackTax", "%lld", llTotalTax)
		.set_format("KuCun", "%lld", llTotalKuCun)
		.set_format("TotalLoseScore", "%lld", llTotalLoseScore)
		.set_format("TotalWinScore", "%lld", llTotalWinScore)
		.set_format("ServerID", "%d", CServerCfg::m_nServerID)
		.set_format("BloodPoolDianKongLoseWin", "%lld", llBloodPoolDianKongWinLose);

	if (db->exec_update(query) == false)
	{
		LOG(ERROR) << "Save Fish Control Data Fail, ServerID: " << CServerCfg::m_nServerID << " TotalWinScore: " << m_nAllTableTotalWinScore
			<< " TotalLoseScore: " << m_nAllTableTotalLoseScore << " KuCun: " << m_nAllTableKuCun << " BlackTax: " << m_nAllTableTotalTax << " BloodPoolDianKongLoseWin: "
			<< m_llBloodPoolDianKongWinLose << " db->get_errno(): " << db->get_errno() << " db->get_error():" << db->get_error();
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
	*/

	MSG_GS2DB_SaveFishControlData data;
	data.llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose;
	data.llTotalKuCun = llTotalKuCun;
	data.llTotalLoseScore = llTotalLoseScore;
	data.llTotalTax = llTotalTax;
	data.llTotalWinScore = llTotalWinScore;
	data.nServerID = CServerCfg::m_nServerID;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CONTROL_DATA, &data, sizeof(MSG_GS2DB_SaveFishControlData));
}

CT_VOID CFishControl::SaveKuCunData()
{
	//如果当前没有人就不记录库存数据
	size_t userCnt = CServerUserManager::get_instance().GetTotalUserCnt();
	CT_WORD androidCnt = CAndroidUserMgr::get_instance().GetOnlineAndroidCount();
	if (userCnt + androidCnt == 0)
	{
		return;
	}

	CT_LONGLONG llTotalKuCun = m_nAllTableKuCun;
	if (CFishServerCfg::m_exchangeRate == 1)
	{

	}
	else if (CFishServerCfg::m_exchangeRate == 2)
	{
		llTotalKuCun = llTotalKuCun / 10;
	}
	else if (CFishServerCfg::m_exchangeRate == 3)
	{
		llTotalKuCun = llTotalKuCun * 10;
	}

	MSG_GS2DB_SaveFishKuCun data;
	data.serverID = CServerCfg::m_nServerID;
	data.llCurKuCun = llTotalKuCun;
	data.llCurTime = time(NULL);
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CURRENT_KUCUN, &data, sizeof(MSG_GS2DB_SaveFishKuCun));
}

CT_VOID CFishControl::StatisticFishInfo()
{
	//先清零
	for (int i = 0; i < MAX_FISH_TYPE; i++)
	{
		m_FishStatisticsInfo[i].ResetData();
	}

	ITableFrame *pTable = NULL;
	ITableFrameSink *pTableSink = NULL;
	VecTableFrame AllTables = CGameTableManager::get_instance().GetTableFrame();
	for (size_t i = 0; i<AllTables.size(); i++)
	{
		pTable = AllTables[i];
		if (pTable)
		{
			pTableSink = pTable->GetTableFrameSink();
			if (pTableSink)
			{
				pTableSink->GetAllPlayerFishStatisticInfo();
			}
		}
	}

	MSG_GS2DB_FishStatisticsInfo saveInfo;
	for (int i = 0; i < MAX_FISH_TYPE; i++)
	{
		if (m_FishStatisticsInfo[i].fishTypeID == 255)
		{
			continue;
		}

		saveInfo.fishTypeID = m_FishStatisticsInfo[i].fishTypeID;
		saveInfo.llDeathCount = m_FishStatisticsInfo[i].llDeathCount;
		saveInfo.llHitCount = m_FishStatisticsInfo[i].llHitCount;
		saveInfo.llTotalBL = m_FishStatisticsInfo[i].llTotalBL;
		saveInfo.llTotalLoseScore = m_FishStatisticsInfo[i].llTotalLoseScore;
		saveInfo.llTotalWinScore = m_FishStatisticsInfo[i].llTotalWinScore;
		saveInfo.ServerID = CServerCfg::m_nServerID;
		memset(saveInfo.strFishName, 0, 128);
		memcpy(saveInfo.strFishName, m_FishStatisticsInfo[i].strFishName.c_str(), m_FishStatisticsInfo[i].strFishName.size());
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_STATISTICSINFO, &saveInfo, sizeof(MSG_GS2DB_FishStatisticsInfo));
	}
}

CT_VOID CFishControl::ReportFishStatisticsInfo(FishStatisticsInfo info[], int arrSize)
{
	for (int i = 0; i < arrSize; i++)
	{
		if (info[i].fishTypeID == 255)
		{
			continue;
		}

		if (info[i].fishTypeID == 30 || info[i].fishTypeID == 31 || info[i].fishTypeID == 32 ||
		info[i].fishTypeID == 33 || info[i].fishTypeID == 35 || info[i].fishTypeID == 36)
		{
			/*for (size_t j = 0; j < info[i].vecSubFishInfo.size(); j++)
			{
				llTotalWin += info[i].vecSubFishInfo[j].llTotalWinScore;
				llTotalLose += info[i].vecSubFishInfo[j].llTotalLoseScore;

				MSG_GS2DB_FishStatisticsInfo saveInfo;
				saveInfo.fishTypeID = info[i].vecSubFishInfo[j].fishTypeID;
				saveInfo.llDeathCount = info[i].vecSubFishInfo[j].llDeathCount;
				saveInfo.llHitCount = info[i].vecSubFishInfo[j].llHitCount;
				saveInfo.llTotalBL = info[i].vecSubFishInfo[j].llTotalBL;
				saveInfo.llTotalLoseScore = info[i].vecSubFishInfo[j].llTotalLoseScore;
				saveInfo.llTotalWinScore = info[i].vecSubFishInfo[j].llTotalWinScore;
				saveInfo.ServerID = CServerCfg::m_nServerID;
				memcpy(saveInfo.strFishName, info[i].vecSubFishInfo[j].strFishName.c_str(), info[i].vecSubFishInfo[j].strFishName.size());
				SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_STATISTICSINFO, &saveInfo, sizeof(MSG_GS2DB_FishStatisticsInfo));
			}*/
		}
		else
		{
			m_FishStatisticsInfo[i].fishTypeID = info[i].fishTypeID;
			m_FishStatisticsInfo[i].llDeathCount += info[i].llDeathCount;
			m_FishStatisticsInfo[i].llHitCount += info[i].llHitCount;
			m_FishStatisticsInfo[i].llTotalBL += info[i].llTotalBL;
			m_FishStatisticsInfo[i].llTotalLoseScore += info[i].llTotalLoseScore;
			m_FishStatisticsInfo[i].llTotalWinScore += info[i].llTotalWinScore;
			m_FishStatisticsInfo[i].ServerID = CServerCfg::m_nServerID;
			m_FishStatisticsInfo[i].strFishName = info[i].strFishName;
		}
	}
}


CT_VOID CFishControl::StatisticFishDeathPro(acl::mysql_manager &dbManager)
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
	LOG(ERROR) << "StatisticFishDeathPro get platform db pool fail.";
	return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
	LOG(ERROR) << "StatisticFishDeathPro get platform db handle fail.";
	return;
	}
	
	acl::query query;
	query.create_sql("select FishTypeID, HitCount, DeathCount, CfgDeathPro from fishstatisticsinfo where ServerID = :ServerID")
		.set_parameter("ServerID", CServerCfg::m_nServerID);
	if (db->exec_select(query) == false)
	{
		LOG(ERROR) << "StatisticFishDeathPro query game kind fail." << "ServerID: " << CServerCfg::m_nServerID << " db->get_errno: " << db->get_errno()
			<< " db->get_error: " << db->get_error();
		pool->put(db);
		return;
	}

	struct tagFishDeathProInfo
	{
		CT_LONGLONG llFishTypeID;
		CT_LONGLONG llHitCount;
		CT_LONGLONG llDeathCount;
		CT_INT32 nCfgDeathPro;
	};

	std::vector<tagFishDeathProInfo> vecPros;
	tagFishDeathProInfo pro;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		pro.llFishTypeID = (CT_LONGLONG)atoll((*row)["FishTypeID"]);
		pro.llHitCount = (CT_LONGLONG)atoll((*row)["HitCount"]);
		pro.llDeathCount = (CT_LONGLONG)atoll((*row)["DeathCount"]);
		pro.nCfgDeathPro = (CT_INT32)atoi((*row)["CfgDeathPro"]);
		vecPros.push_back(pro);
	}

	CT_DOUBLE dHitCount, dDeahtCount, dTemp;
	CT_LONGLONG llTemp;
	CT_INT32 nBigCnt = 0, nSmallCnt = 0, nEqualCnt = 0;
	for (size_t i = 0; i < vecPros.size(); i++)
	{
		dHitCount = vecPros[i].llHitCount;
		dDeahtCount = vecPros[i].llDeathCount;
		dTemp = dDeahtCount / dHitCount;
		dTemp = dTemp * 10000;
		llTemp = dTemp;

		if (llTemp > vecPros[i].nCfgDeathPro)
		{
			nBigCnt++;
			LOG(INFO) << "鱼种: " << vecPros[i].llFishTypeID << " 实际捕获概率: " << llTemp << " 高于配置捕获概率: " << vecPros[i].nCfgDeathPro
				<< " 高出: " << llTemp - vecPros[i].nCfgDeathPro;
		}
		else if (llTemp < vecPros[i].nCfgDeathPro)
		{
			nSmallCnt++;
			LOG(INFO) << "鱼种: " << vecPros[i].llFishTypeID << " 实际捕获概率: " << llTemp << " 低于配置捕获概率: " << vecPros[i].nCfgDeathPro
				<< "低出: " << vecPros[i].nCfgDeathPro - llTemp;
		}
		else
		{
			nEqualCnt++;
		}
	}

	LOG(INFO) << "实际捕获概率 高于 配置捕获概率的次数: " << nBigCnt << " 实际捕获概率 低于 配置捕获概率的次数: " << nSmallCnt << " 相等的次数: " << nEqualCnt;

	db->free_result();
	pool->put(db);
}

CT_BOOL CFishControl::ReadFishControlData(acl::mysql_manager &dbManager)
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(ERROR) << "ReadFishControlData get platform db pool fail.";
		return CT_FALSE;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(ERROR) << "ReadFishControlData get platform db handle fail.";
		return CT_FALSE;
	}

	acl::query query;
	query.create_sql("select TotalWinScore, TotalLoseScore, KuCun, BlackTax, BloodPoolDianKongLoseWin from stock_control_data where ServerID = :ServerID")
		.set_parameter("ServerID", CServerCfg::m_nServerID);
	if (db->exec_select(query) == false)
	{
		LOG(ERROR) << "ReadFishControlData query game kind fail." << "ServerID: " << CServerCfg::m_nServerID << " db->get_errno: " << db->get_errno()
			<< " db->get_error: " << db->get_error();
		pool->put(db);
		return CT_FALSE;
	}

	if (db->length() == 0)
	{
		//初次启动
		m_nAllTableTotalWinScore = 0;
		m_nAllTableTotalLoseScore = 0;
		m_nAllTableKuCun = 0;
		m_nAllTableTotalTax = 0;
		m_llBloodPoolDianKongWinLose = 0;
	}
	else if(db->length() == 1)
	{
		const acl::db_row* row = (*db)[0];
		CT_LONGLONG llTotalTax = atoll((*row)["BlackTax"]);;
		CT_LONGLONG llTotalKuCun = atoll((*row)["KuCun"]);
		CT_LONGLONG llTotalLoseScore = atoll((*row)["TotalLoseScore"]);
		CT_LONGLONG llTotalWinScore = atoll((*row)["TotalWinScore"]);
		CT_LONGLONG llBloodPoolDianKongWinLose = atoll((*row)["BloodPoolDianKongLoseWin"]);
		if (CFishServerCfg::m_exchangeRate == 1)
		{
			m_nAllTableTotalWinScore = llTotalWinScore ;
			m_nAllTableTotalLoseScore = llTotalLoseScore ;
			m_nAllTableKuCun = llTotalKuCun;
			m_nAllTableTotalTax = llTotalTax;
			m_llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose;
		}
		else if (CFishServerCfg::m_exchangeRate == 2)
		{
			m_nAllTableTotalWinScore = llTotalWinScore * 10;
			m_nAllTableTotalLoseScore = llTotalLoseScore * 10;
			m_nAllTableKuCun = llTotalKuCun * 10;
			m_nAllTableTotalTax = llTotalTax * 10;
			m_llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose * 10;
		}
		else if (CFishServerCfg::m_exchangeRate == 3)
		{
			m_nAllTableTotalWinScore = llTotalWinScore / 10;
			m_nAllTableTotalLoseScore = llTotalLoseScore / 10;
			m_nAllTableKuCun = llTotalKuCun / 10;
			m_nAllTableTotalTax = llTotalTax / 10;
			m_llBloodPoolDianKongWinLose = llBloodPoolDianKongWinLose / 10;
		}
	}
	else
	{
		LOG(ERROR) << "ReadFishControlData return DataSet count: " << db->length();
		db->free_result();
		pool->put(db);
		return CT_FALSE;
	}

	LOG(INFO) << "m_nAllTableTotalWinScore: " << m_nAllTableTotalWinScore << " m_nAllTableTotalLoseScore" << m_nAllTableTotalLoseScore
		<< " m_nAllTableKuCun" << m_nAllTableKuCun << " m_nAllTableTotalTax" << m_nAllTableTotalTax << " m_llBloodPoolDianKongWinLose: "<< m_llBloodPoolDianKongWinLose;
	db->free_result();
	pool->put(db);

	m_llKuCunCaiYang = m_nAllTableKuCun;

	sKuCunCaiYangInfo cyInfo;
	cyInfo.nCaiYangTime = time(NULL);
	cyInfo.llKuCuCaiYang = m_nAllTableKuCun;
	if (CFishServerCfg::IsSaveCurrentKuCun == 1)
	{
		//配置中允许保存实时库存才把数据加入,否则内存一直会增加
		m_listKuCunCaiYang.push_back(cyInfo);
	}

	//关闭日志
	//WrtLog(CServerCfg::m_nServerID, 1, "%lld", m_nAllTableKuCun);

	return CT_TRUE;
}
