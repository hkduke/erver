
#include "IGameOtherData.h"
#include "stdafx.h"
#include <stdio.h>
#include "GameProcess.h"
#include "ITableFrame.h"
#include <time.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "acl_cpp/lib_acl.hpp"
#include "acl_cpp/stdlib/json.hpp"
#include "Utility.h"
#include "GlobalEnum.h"
#include "MSG_ermj.h"
#include "GameLogic.h"
#include "testlog.h"
#include <bitset>
#include <cmath>
#include <iostream>
#include <random>
//#include <glog_linux/glog/logging.h>

//构造函数
CGameProcess::CGameProcess()
{
	m_dwBanker = INVALID_CHAIR_ID;
	m_dwWinerChaifID = INVALID_CHAIR_ID;
}

//析构函数
CGameProcess::~CGameProcess()
{

}

//复位桌子
CT_VOID CGameProcess::ResetTableData_PerGameRound()
{
	//清除所有定时器
	ClearAllTimer();
	//清除牌堆信息
	memset(m_cbRepertoryCard, INVALID_CARD_DATA, sizeof(m_cbRepertoryCard));
	m_cbLeftCardCount = 0;
	m_dwCurRepertoryCardIndex = 0;

	//清除玩家相关信息
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		it->second.clearPerGameRound();
	}

	m_dwTingCardPlayerCnt = 0;
	m_dwCurOutCardPlayer = 0;
	m_cbZhuaCardData = INVALID_CARD_DATA;
	m_dwCurZhuaCardPlayer = INVALID_CHAIR_ID;
	m_dwAutoOutCardPlayer = INVALID_CHAIR_ID;
	m_dWAutoOutCardData = INVALID_CARD_DATA;
	m_cbCurOutCardData = INVALID_CARD_DATA;
    m_cbCurState = STATE_DEFAUT;
	m_cbAllPlayerOutCardCount = 0;
	m_bCanTianTingBanker = CT_FALSE;
	m_bCanTianTingXian = CT_FALSE;
	m_cbCancelTing_OutTime_OutCard = INVALID_CARD_DATA;
	m_dwCurGangPlayer = INVALID_CHAIR_ID;
	m_dwShouldOutCardPlayer = INVALID_CHAIR_ID;
    m_bXianPlayerFirstZhuaCard = CT_TRUE;
    m_cbLastZhuaCardData = INVALID_CARD_DATA;
    m_bZhuangFirstOutCard = CT_TRUE;
	m_cbAllPlayerZhuaCardCount = 0;
}

CT_VOID CGameProcess::SetHuTypeFanshu(CT_BYTE cbPlayerHuType[],CT_BYTE cbMaxHuTypeCount ,CT_DWORD dwChairID)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "SetHuTypeFanshu 通过椅子ID ："<< dwChairID << "找不到玩家";
		return;
	}
	if(it->second.bHu == CT_FALSE)
	{
		LOG(ERROR) << "SetHuTypeFanshu bHu:为CT_FALSE";
		return;
	}
	if(it->second.llHuCardType == -1 || it->second.wHuCardFan <= 0)
	{
		LOG(ERROR) << "SetHuTypeFanshu llHuCardType:"<< it->second.llHuCardType << " it->second.wHuCardFan:" <<it->second.wHuCardFan;
		return;
	}

	EnumFlag<HUTYPES> eFlag;
	eFlag.SetData(it->second.llHuCardType);

	//在gameprocess中判断人和
	//gamelogic中把人和判断成了地胡
	if(m_cbAllPlayerOutCardCount == 1 && dwChairID != m_dwBanker && m_cbAllPlayerZhuaCardCount == 0)
	{
		if(eFlag.TestFlag(HU_TYPE_DI_HU) && eFlag.TestFlag(HU_TYPE_REN_HU))
		{
			it->second.wHuCardFan -= 88;
		}
		else if(!eFlag.TestFlag(HU_TYPE_DI_HU) && !eFlag.TestFlag(HU_TYPE_REN_HU))
		{
			it->second.wHuCardFan += 88;
		}

		eFlag.ResetFlag(HU_TYPE_DI_HU);
		eFlag.SetFlag(HU_TYPE_REN_HU);
	}

	//LOG(ERROR) << "TTTTTT 玩家ID: "<< it->second.dwUserID << "胡牌类型:" << it->second.llHuCardType;
	if(eFlag.TestFlag(HU_TYPE_DAN_DIAO))
	{
		cbPlayerHuType[HU_TYPE_DAN_DIAO] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_KAN_ZHANG))
	{
		cbPlayerHuType[HU_TYPE_KAN_ZHANG] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_BIAN_ZHANG))
	{
		cbPlayerHuType[HU_TYPE_BIAN_ZHANG] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_MING_GANG))
	{
		cbPlayerHuType[HU_TYPE_MING_GANG] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_YAO_JIU_KE))
	{
		cbPlayerHuType[HU_TYPE_YAO_JIU_KE] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_LAO_SHAO_FU))
	{
		cbPlayerHuType[HU_TYPE_LAO_SHAO_FU] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_LIAN_LIU))
	{
		cbPlayerHuType[HU_TYPE_LIAN_LIU] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_BAN_GAO))
	{
		cbPlayerHuType[HU_TYPE_YI_BAN_GAO] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_BAO_TING))
	{
		cbPlayerHuType[HU_TYPE_BAO_TING] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_YAO_JIU_TOU))
	{
		cbPlayerHuType[HU_TYPE_YAO_JIU_TOU] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_ER_WU_BA_JIANG))
	{
		cbPlayerHuType[HU_TYPE_ER_WU_BA_JIANG] = 1;
	}
	if(eFlag.TestFlag(HU_TYPE_DUAN_YAO))
	{
		cbPlayerHuType[HU_TYPE_DUAN_YAO] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_AN_GANG))
	{
		cbPlayerHuType[HU_TYPE_AN_GANG] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_SHUANG_AN_KE))
	{
		cbPlayerHuType[HU_TYPE_SHUANG_AN_KE] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_SI_GUI_YI))
	{
		cbPlayerHuType[HU_TYPE_SI_GUI_YI] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_PING_HU))
	{
		cbPlayerHuType[HU_TYPE_PING_HU] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_MEN_QIAN_QING))
	{
		cbPlayerHuType[HU_TYPE_MEN_QIAN_QING] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_MEN_FENG_KE))
	{
		cbPlayerHuType[HU_TYPE_MEN_FENG_KE] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_QUAN_FENG_KE))
	{
		cbPlayerHuType[HU_TYPE_QUAN_FENG_KE] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_JIAN_KE))
	{
		cbPlayerHuType[HU_TYPE_JIAN_KE] = 2;
	}
	if(eFlag.TestFlag(HU_TYPE_LI_ZHI))
	{
		cbPlayerHuType[HU_TYPE_LI_ZHI] = 4;
	}
	if(eFlag.TestFlag(HU_TYPE_HU_JUE_ZHANG))
	{
		cbPlayerHuType[HU_TYPE_HU_JUE_ZHANG] = 4;
	}
	if(eFlag.TestFlag(HU_TYPE_SHUANG_MING_GANG))
	{
		cbPlayerHuType[HU_TYPE_SHUANG_MING_GANG] = 4;
	}
	if(eFlag.TestFlag(HU_TYPE_BU_QIU_REN))
	{
		cbPlayerHuType[HU_TYPE_BU_QIU_REN] = 4;
	}
	if(eFlag.TestFlag(HU_TYPE_QUAN_DAI_YAO))
	{
		cbPlayerHuType[HU_TYPE_QUAN_DAI_YAO] = 4;
	}
	if(eFlag.TestFlag(HU_TYPE_SHUANG_JIAN_KE))
	{
		cbPlayerHuType[HU_TYPE_SHUANG_JIAN_KE] = 6;
	}
	if(eFlag.TestFlag(HU_TYPE_SHUANG_AN_GANG))
	{
		cbPlayerHuType[HU_TYPE_SHUANG_AN_GANG] = 6;
	}
	if(eFlag.TestFlag(HU_TYPE_QUAN_QIU_REN))
	{
		cbPlayerHuType[HU_TYPE_QUAN_QIU_REN] = 6;
	}
	if(eFlag.TestFlag(HU_TYPE_HUN_YI_SE))
	{
		cbPlayerHuType[HU_TYPE_HUN_YI_SE] = 6;
	}
	if(eFlag.TestFlag(HU_TYPE_PENG_PENG_HU))
	{
		cbPlayerHuType[HU_TYPE_PENG_PENG_HU] = 6;
	}
	if(eFlag.TestFlag(HU_TYPE_QIANG_GANG_HU))
	{
		cbPlayerHuType[HU_TYPE_QIANG_GANG_HU] = 8;
	}
	if(eFlag.TestFlag(HU_TYPE_GANG_SHANG_HUA))
	{
		cbPlayerHuType[HU_TYPE_GANG_SHANG_HUA] = 8;
	}
	if(eFlag.TestFlag(HU_TYPE_HAI_DI_LAO_YUE))
	{
		cbPlayerHuType[HU_TYPE_HAI_DI_LAO_YUE] = 8;
	}
	if(eFlag.TestFlag(HU_TYPE_MIAO_SHOU_HUI_CHUN))
	{
		cbPlayerHuType[HU_TYPE_MIAO_SHOU_HUI_CHUN] = 8;
	}
	if(eFlag.TestFlag(HU_TYPE_SAN_FENG_KE))
	{
		cbPlayerHuType[HU_TYPE_SAN_FENG_KE] = 12;
	}
	if(eFlag.TestFlag(HU_TYPE_XIAO_YU_5))
	{
		cbPlayerHuType[HU_TYPE_XIAO_YU_5] = 12;
	}
	if(eFlag.TestFlag(HU_TYPE_DA_YU_5))
	{
		cbPlayerHuType[HU_TYPE_DA_YU_5] = 12;
	}
	if(eFlag.TestFlag(HU_TYPE_TIAN_TING))
	{
		cbPlayerHuType[HU_TYPE_TIAN_TING] = 16;
	}
	if(eFlag.TestFlag(HU_TYPE_SAN_AN_KE))
	{
		cbPlayerHuType[HU_TYPE_SAN_AN_KE] = 16;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SAN_BU_GAO))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SAN_BU_GAO] = 16;
	}
	if(eFlag.TestFlag(HU_TYPE_QING_LONG))
	{
		cbPlayerHuType[HU_TYPE_QING_LONG] = 16;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SAN_JIE_GAO))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SAN_JIE_GAO] = 24;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SAN_TONG_SHUN))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SAN_TONG_SHUN] = 24;
	}
	if(eFlag.TestFlag(HU_TYPE_QING_YI_SE))
	{
		cbPlayerHuType[HU_TYPE_QING_YI_SE] = 24;
	}
	if(eFlag.TestFlag(HU_TYPE_QI_DUI))
	{
		cbPlayerHuType[HU_TYPE_QI_DUI] = 24;
	}
	if(eFlag.TestFlag(HU_TYPE_HUN_YAO_JIU))
	{
		cbPlayerHuType[HU_TYPE_HUN_YAO_JIU] = 32;
	}
	if(eFlag.TestFlag(HU_TYPE_SAN_GANG))
	{
		cbPlayerHuType[HU_TYPE_SAN_GANG] = 32;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SI_BU_GAO))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SI_BU_GAO] = 32;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SI_JIE_GAO))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SI_JIE_GAO] = 48;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SI_TONG_SHUN))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SI_TONG_SHUN] = 48;
	}
	if(eFlag.TestFlag(HU_TYPE_YI_SE_SHUANG_LONG))
	{
		cbPlayerHuType[HU_TYPE_YI_SE_SHUANG_LONG] = 64;
	}
	if(eFlag.TestFlag(HU_TYPE_SI_AN_KE))
	{
		cbPlayerHuType[HU_TYPE_SI_AN_KE] = 64;
	}
	if(eFlag.TestFlag(HU_TYPE_ZI_YI_SE))
	{
		cbPlayerHuType[HU_TYPE_ZI_YI_SE] = 64;
	}
	if(eFlag.TestFlag(HU_TYPE_XIAO_SAN_YUAN))
	{
		cbPlayerHuType[HU_TYPE_XIAO_SAN_YUAN] = 64;
	}
	if(eFlag.TestFlag(HU_TYPE_XIAO_SI_XI))
	{
		cbPlayerHuType[HU_TYPE_XIAO_SI_XI] = 64;
	}
	if(eFlag.TestFlag(HU_TYPE_BAI_WAN_SHI))
	{
		cbPlayerHuType[HU_TYPE_BAI_WAN_SHI] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_REN_HU))
	{
		cbPlayerHuType[HU_TYPE_REN_HU] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_DI_HU))
	{
		cbPlayerHuType[HU_TYPE_DI_HU] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_TIAN_HU))
	{
		cbPlayerHuType[HU_TYPE_TIAN_HU] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_LIAN_QI_DUI))
	{
		cbPlayerHuType[HU_TYPE_LIAN_QI_DUI] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_SI_GANG))
	{
		cbPlayerHuType[HU_TYPE_SI_GANG] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_JIU_LIAN_BAO_DENG))
	{
		cbPlayerHuType[HU_TYPE_JIU_LIAN_BAO_DENG] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_DA_SAN_YUAN))
	{
		cbPlayerHuType[HU_TYPE_DA_SAN_YUAN] = 88;
	}
	if(eFlag.TestFlag(HU_TYPE_DA_SI_XI))
	{
		cbPlayerHuType[HU_TYPE_DA_SI_XI] = 88;
	}
	//最后一位放自摸
	if(it->second.bIsSuanZiMo)
	{
		cbPlayerHuType[MAX_FNASHU - 2] = 1;
	}
	else
	{
		cbPlayerHuType[MAX_FNASHU - 2] = 0;
	}
	if(it->second.wJiaBei > 0)
	{
        cbPlayerHuType[MAX_FNASHU - 1] = it->second.wJiaBei;
	}
	else
	{
		cbPlayerHuType[MAX_FNASHU - 1] = 0;
	}
}

CT_VOID CGameProcess::ClearAllTimer()
{
	m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
	m_pGameDeskPtr->KillGameTimer(IDI_START_GAME);
	m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_OUTCARD);
	m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD);
	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY);
	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_OUT_CARD_AFTER_TING);
	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_HU_CARD_AFTER_JIABEI);
	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_JIESAN_DESK);
}

//游戏结束
CT_VOID CGameProcess::OnEventGameEnd(CT_DWORD wChairID, GameEndTag GETag)
{
    //本局结束，删除监控定时器
    m_pGameDeskPtr->KillGameTimer(IDI_AUTO_JIESAN_DESK);

    std::string strTime = Utility::GetTimeNowString();
    m_pGameDeskPtr->ConcludeGame(0, strTime.c_str());
    //m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_End);

    CMD_S_GameEnd data;
    if(wChairID != INVALID_CHAIR)
	{
		data.bIsNoWin = CT_FALSE;
	}
	else
	{
		data.bIsNoWin = CT_TRUE;
	}

    data.wBanker = m_dwBanker;
    data.wWinner = wChairID;
    memset(data.dCardType, 0, sizeof(data.dCardType));
    if(wChairID != INVALID_CHAIR)
	{
		SetHuTypeFanshu(data.dCardType, MAX_FNASHU, wChairID);
	}

    for (int i = 0; i < MAX_FNASHU; i++)
    {
        if(data.dCardType[i] > 0)
        {
            LOG_IF(WARNING, PRINT_LOG_INFO) << "胡的牌型: "<< i << " 番数: "<< (int)data.dCardType[i];
            //LOG_IF(WANING, PRINT_LOG_INFO) << "胡的牌型: "<< i << " 番数: "<< (int)data.dCardType[i];
        }
    }

	data.bJiangId = 0;

    CT_LONGLONG llTax = 0;
    //llWinerAddScore:赢家扣税后赢的钱 llLoserAddScore:输家应该输的钱 llWinScore:赢家扣税前赢的钱
    CT_LONGLONG llWinerAddScore = 0, llLoserAddScore = 0, llWinScore = 0;
    if(wChairID != INVALID_CHAIR)
	{
		MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
		if(it != m_mapChairInfo.end())
		{
			data.bHuCardId = it->second.cbHuCard;
			if(it->second.bZiMo)
			{
				//如果是自摸胡牌，先从赢家手中删除赢的玩家的胡牌：客户端要求
				m_GameLogic.RemoveCard(it->second.cbHandCardIndex, data.bHuCardId);
			}
		}
		else
		{
			data.bHuCardId = INVALID_CARD_DATA;
		}

		CT_LONGLONG llWinFan = it->second.wHuCardFan;
		CT_LONGLONG llCellScore = m_pGameDeskPtr->GetGameCellScore();
		llWinScore = llWinFan * llCellScore;
		CT_DWORD  cbBei = std::pow(2, it->second.wJiaBei);
		//赢家赢的钱
		llWinScore = llWinScore * cbBei;

		//判断赢家的钱是否超过了进场时的金币
		if(llWinScore > it->second.llInitScore)
        {
		    llWinScore = it->second.llInitScore;
		    data.cbWinLoseMax = 1;
        }

		//判断输家的钱够不够
		for(MapChairInfo::iterator itLoser = m_mapChairInfo.begin(); itLoser != m_mapChairInfo.end(); itLoser++)
		{
			if(itLoser->first == wChairID)
			{
				continue;
			}

			//如果输家的钱不够，输家剩余的钱就是赢家赢的钱
			CT_LONGLONG llLoserScore = m_pGameDeskPtr->GetUserScore(itLoser->first);
			if(llLoserScore <= llWinScore)
			{
				llWinScore = llLoserScore;
                data.cbWinLoseMax = 1;
			}
			break;
		}

		for(MapChairInfo::iterator itScore = m_mapChairInfo.begin(); itScore != m_mapChairInfo.end(); itScore++)
		{
			if(itScore->first == wChairID)
			{
				//计算赢家税收
				llTax = m_pGameDeskPtr->CalculateRevenue(wChairID,llWinScore);
				itScore->second.llTotalTax += llTax;
				itScore->second.llCurSocre += (llWinScore - llTax);
				llWinerAddScore = llWinScore - llTax;
			}
			else
			{
				//输家
				itScore->second.llCurSocre -= llWinScore;
				llLoserAddScore = llWinScore * (-1);
			}
		}

		CT_DOUBLE dWinerScore = llWinerAddScore;
		dWinerScore = dWinerScore / 100;
		CT_DOUBLE dLoserScore = llLoserAddScore;
		dLoserScore = dLoserScore / 100;

		if(wChairID == 0)
		{
			data.lScore[0] = dWinerScore;
			data.lScore[1] = dLoserScore;
		}
		else if(wChairID == 1)
		{
			data.lScore[0] = dLoserScore;
			data.lScore[1] = dWinerScore;
		}
		else
		{
			LOG(ERROR) << "error:" << wChairID;
		}
	}
	else
	{
		data.bHuCardId = INVALID_CARD_DATA;
		data.lScore[0] = 0;
		data.lScore[1] = 0;
    }

	for(MapChairInfo::iterator itAll = m_mapChairInfo.begin(); itAll != m_mapChairInfo.end(); itAll++)
	{
		CT_BYTE cbHandCardIndex = 0;
		for(int i = 0; i < MAX_INDEX; i++)
		{
			if(itAll->second.cbHandCardIndex[i] > 0)
			{
				for(int j = 0; j < itAll->second.cbHandCardIndex[i]; j++)
				{
					CT_BYTE cbCardData = m_GameLogic.SwitchToCardData(i);
					data.bCurHandCardData[itAll->first][cbHandCardIndex] = cbCardData;
					cbHandCardIndex++;
				}
			}
		}

		CT_BYTE cbActionIndex = 0;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(itAll->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
			{
				data.cbCurActionCardData[itAll->first][cbActionIndex].wOperateCode = itAll->second.weaveItem[i].cbWeaveKind;
				data.cbCurActionCardData[itAll->first][cbActionIndex].cbCardId = itAll->second.weaveItem[i].cbCenterCard;
				data.cbCurActionCardData[itAll->first][cbActionIndex].wProvideUser = itAll->second.weaveItem[i].wProvideUser;
				cbActionIndex++;
			}
		}
	}

	if(wChairID != INVALID_CHAIR)
	{
		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == wChairID)
			{
				SendTableData(p, SUB_S_GAME_END, &data, sizeof(data), true);
			}
			else
			{
				SendTableData(p, SUB_S_GAME_END, &data, sizeof(data), false);
			}
		}
	}
	else
	{
		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == 0)
			{
				SendTableData(p, SUB_S_GAME_END, &data, sizeof(data), true);
			}
			else
			{
				SendTableData(p, SUB_S_GAME_END, &data, sizeof(data), false);
			}
		}
	}

	for(MapChairInfo::iterator itit = m_mapChairInfo.begin(); itit != m_mapChairInfo.end(); itit++)
	{
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(data.cbCurActionCardData[itit->first][i].wOperateCode != WIK_NULL_MY)
			{
				LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家ID: " << m_pGameDeskPtr->GetUserID(itit->first) << " 组合类型:" << data.cbCurActionCardData[itit->first][i].wOperateCode
				<< "牌:" <<(int)data.cbCurActionCardData[itit->first][i].cbCardId;
			}
		}
	}

	m_dwWinerChaifID = wChairID;

	RecordScoreInfo scoreInfo[GAME_PLAYER];
	for(MapChairInfo::iterator itRecord = m_mapChairInfo.begin(); itRecord != m_mapChairInfo.end(); itRecord++)
	{
		if(wChairID != INVALID_CHAIR)
		{
			//如果不是流局就写分
			ScoreInfo  ScoreData;
			ScoreData.dwUserID = itRecord->second.dwUserID;
			ScoreData.bBroadcast = true;
			if(itRecord->first == wChairID)
			{
				ScoreData.llScore = llWinerAddScore;
				ScoreData.llRealScore = llWinerAddScore;
			}
			else
			{
				ScoreData.llScore = llLoserAddScore;
				ScoreData.llRealScore = llLoserAddScore;
			}
			//写分
			m_pGameDeskPtr->WriteUserScore(itRecord->first, ScoreData);
		}

		//写牌局回放
		scoreInfo[itRecord->first].dwUserID = itRecord->second.dwUserID;
		scoreInfo[itRecord->first].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(itRecord->first);
		scoreInfo[itRecord->first].cbStatus = 1;
		scoreInfo[itRecord->first].llSourceScore = itRecord->second.llInitScore;
		if(wChairID == itRecord->first)
			scoreInfo[itRecord->first].iScore = llWinScore;
		else
			scoreInfo[itRecord->first].iScore = llLoserAddScore;

		if(itRecord->first == wChairID)
			scoreInfo[itRecord->first].dwRevenue = (CT_DWORD)llTax;
		else
			scoreInfo[itRecord->first].dwRevenue = 0;
	}
	//记录牌局录像
	m_pGameDeskPtr->RecordRawInfo(scoreInfo, GAME_PLAYER, NULL, 0, 0, 0, 0, 0, 0, 0);

	//
	if (m_PrivateTableInfo.dwRoomNum == 0)
    {
        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            OnUserLeft(i, false);
            if (!m_pGameDeskPtr->IsAndroidUser(i))
                m_pGameDeskPtr->ClearTableUser(i, false);
        }

        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 1000, 0);
    }
    else
    {
        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            m_pGameDeskPtr->SetUserStatus(i, sSit);
        }

        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 15000, 0);
    }

    //牌局结束删除所有定时器
	ClearAllTimer();
	LOG_IF(WARNING, PRINT_LOG_INFO) << "游戏结束";
}

//发送场景
CT_VOID CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if(m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Start)
	{
		MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
		if(it != m_mapChairInfo.end())
		{
			//断线重连流程
			DisReConnect(dwChairID);
			return;
		}
		else
		{
			LOG(ERROR) << "玩家: "<<dwChairID << " 重连,但是找不到了";
		}
	}
	else if(m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Free)
	{
		CMD_S_StatusFree StatusFree;
		memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
		StatusFree.dCellScore = m_pGameDeskPtr->GetGameCellScore() * 0.01f;
		StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

		//发送数据
		SendTableData(dwChairID, SUB_S_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),true);
	}
	else if(m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		LOG(ERROR) << "玩家: "<<m_pGameDeskPtr->GetUserID(dwChairID) << " 断线重连上来的时候,游戏状态为END";
	}
}

//定时器事件
CT_VOID CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch(dwTimerID)
	{
		case IDI_START_GAME:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_START_GAME);
				OnTimer_StartGame();
				break;
			}
		case IDI_OUT_CARD:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
				OnTimer_PlayerOutCard();
				break;
			}
		case IDI_OPERATE_TIME_OUT_AFTER_OUTCARD:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_OUTCARD);
				OnTimer_PlayerOperateTimeOut_AfterOutCard(dwParam);
				break;
			}
		case IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD);
				OnTimer_PlayerOperateTimeOut_AfterZhuaCard(dwParam);
				break;
			}
		case IDI_AUTO_READY:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY);
				OnTimer_AutoReady();
				break;
			}
	    case IDI_AUTO_OUT_CARD_AFTER_TING:
            {
            	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_OUT_CARD_AFTER_TING);
            	OnTimer_AutoOutCardAfterTing();
                break;
            }
		case IDI_AUTO_HU_CARD_AFTER_JIABEI:
			{
				m_pGameDeskPtr->KillGameTimer(IDI_AUTO_HU_CARD_AFTER_JIABEI);
				OnTimer_AutoHuCardAfterJiaBei(dwParam);
				break;
			}
	    case IDI_AUTO_JIESAN_DESK:
            {
                m_pGameDeskPtr->KillGameTimer(IDI_AUTO_JIESAN_DESK);
                OnTimer_SystemJieSanDesk();
                break;
            }
		case IDI_TICK_USER:
			{
                m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER);
			    if (m_PrivateTableInfo.dwRoomNum == 0)
                {
                    for (int i = 0; i < GAME_PLAYER; ++i)
                    {
                        if (m_pGameDeskPtr->IsAndroidUser(i))
                            m_pGameDeskPtr->ClearTableUser(i, false);
                    }
                }
                else
                {
                    if (m_pGameDeskPtr->GetGameRoundPhase() != en_GameRound_Start)
                    {
                        for (int i = 0; i < GAME_PLAYER; ++i)
                        {
                            if (m_pGameDeskPtr->IsExistUser(i) && m_pGameDeskPtr->GetUserStatus(i) != sReady)
                                m_pGameDeskPtr->ClearTableUser(i, true);
                        }
                    }
                }

				break;
			}
		default:
			LOG(ERROR) << "未处理的定时器:" <<dwTimerID;
			break;
	}
}

CT_VOID CGameProcess::OnTimer_SystemJieSanDesk()
{
	LOG(ERROR) << "系统自动解散桌子，桌子内的玩家:" << m_pGameDeskPtr->GetUserID(0) << " 和玩家: "<< m_pGameDeskPtr->GetUserID(1);
	OnEventGameEnd(INVALID_CHAIR,GER_NORMAL);
}

CT_VOID CGameProcess::OnTimer_AutoHuCardAfterJiaBei(CT_DWORD dwJiaBeiPlayer)
{
	//默认胡牌
	CMD_C_OperateTing ting;
	ting.dwUserID = m_pGameDeskPtr->GetUserID(dwJiaBeiPlayer);
	ting.wOperateCode = 0;
	ting.wOperateCode |= WIK_HU_MY;
	MapChairInfo::iterator it = m_mapChairInfo.find(dwJiaBeiPlayer);
	if(it != m_mapChairInfo.end())
	{
		if(it->second.dwActionProvider == dwJiaBeiPlayer)
		{
			ting.wOperateCode |= WIK_HU_ZI_MO_MY;
		}
		else
		{
			ting.wOperateCode |= WIK_HU_FANG_PAO_MY;
		}

		OnMsg_PlayerJiaBei(dwJiaBeiPlayer, &ting, sizeof(ting));
	}
	else
	{
		LOG(ERROR) << "OnTimer_AutoHuCardAfterJiaBei 通过椅子ID:" << dwJiaBeiPlayer << " 找不到玩家";
	}

}

CT_VOID CGameProcess::OnTimer_AutoOutCardAfterTing()
{
    MapChairInfo::iterator it = m_mapChairInfo.find(m_dwAutoOutCardPlayer);
    if(it == m_mapChairInfo.end())
    {
        LOG(ERROR) << "OnTimer_AutoOutCardAfterTing 找不到自动出牌的玩家: "<< m_dwAutoOutCardPlayer;
        return;
    }

    if(m_GameLogic.IsValidCard(m_dWAutoOutCardData) == CT_FALSE)
    {
        LOG(ERROR) << "OnTimer_AutoOutCardAfterTing 自动出的牌: " << (CT_INT32)m_dWAutoOutCardData << " 无效";
        return;
    }

    if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, m_dWAutoOutCardData) == CT_FALSE)
    {
        LOG(ERROR) << "OnTimer_AutoOutCardAfterTing 自动出牌的时候, 玩家: "<< it->second.dwUserID << " 没有牌: "<< (CT_INT32)m_dWAutoOutCardData;
        return;
    }

    PlayerOutCard(m_dwAutoOutCardPlayer, m_dWAutoOutCardData, 0, CT_TRUE);
}

CT_VOID CGameProcess::OnTimer_StartGame()
{
    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家: "<< m_pGameDeskPtr->GetUserID(m_dwBanker)<< " 椅子ID:"<< m_dwBanker <<" 出牌:" << (int)m_cbZhuaCardData << " 开始游戏";

    //判断庄家能否暗杠 自摸胡
    if(IsHaveAction_AfterZhuaCard(m_dwBanker,m_cbZhuaCardData))
    {
    	TipPlayerOutCard(m_dwBanker, m_cbZhuaCardData, CT_FALSE);
    	//庄家起牌就有行为，判断是否能天听
    	MapChairInfo::iterator it = m_mapChairInfo.find(m_dwBanker);
    	if(it != m_mapChairInfo.end())
		{
    		if(it->second.cbPlayerAction & WIK_TING_MY)
			{
    			//庄家能天听
    			m_bCanTianTingBanker = CT_TRUE;
			}
		}

		m_startOperateTime = Utility::GetTick();
		m_cbCurState = STATE_WAIT_SEND_OPERATE;
		//抓牌后提示玩家行为
		TipPlayerAction(m_dwBanker, m_cbZhuaCardData, Trigger_ZhuaCard);
    }
    else
	{
		TipPlayerOutCard(m_dwBanker, m_cbZhuaCardData, CT_TRUE ,CT_TRUE);
	}
}

CT_VOID CGameProcess::OnTimer_PlayerOperateTimeOut_AfterZhuaCard(CT_DWORD dwParam)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwParam);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "通过椅子ID：" << dwParam << " 找不到玩家对象,OnTimer_PlayerOperateTimeOut_AfterZhuaCard";
		return;
	}

	if(it->second.cbPlayerAction & WIK_HU_MY && it->second.cbPlayerAction & WIK_HU_ZI_MO_MY)
	{
		//如果抓牌触发了胡行为，那么超时就直接胡
		CMD_C_OperateSend data;
		data.dwUserID = it->second.dwUserID;
		data.cbCardId = INVALID_CARD_DATA;
		data.wOperateCode = 0;
		data.wOperateCode |= WIK_HU_MY;
		data.wOperateCode |= WIK_HU_ZI_MO_MY;
		OnMsg_PlayerOperate_ByZhuaCard(it->first, &data, sizeof(data));
	}
	else
	{
		//如果抓牌触发了行为，但是不包括胡，那么超时就直接出牌
		OnTimer_PlayerOutCard();
		/*CMD_C_OperateSend data;
		data.dwUserID = it->second.dwUserID;
		data.cbCardId = INVALID_CARD_DATA;
		data.wOperateCode = WIK_NULL_MY_TIMEOUT;
		OnMsg_PlayerOperate_ByZhuaCard(it->first, &data, sizeof(data));*/
	}
}

CT_VOID CGameProcess::OnTimer_PlayerOperateTimeOut_AfterOutCard(CT_DWORD dwParam)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwParam);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << " 通过椅子ID: "<< dwParam << " 找不到玩家。。";
		return;
	}

	if(it->second.cbPlayerAction & WIK_HU_MY && it->second.cbPlayerAction & WIK_HU_FANG_PAO_MY)
	{
		CMD_C_OperateOut op;
		op.wOperateCode = 0;
		op.wOperateCode |= WIK_HU_MY;
		op.wOperateCode |= WIK_HU_FANG_PAO_MY;
		op.dwUserID = m_pGameDeskPtr->GetUserID(dwParam);
		OnMsg_PlayerOperate_ByOutCard(dwParam, &op, sizeof(op));
	}
	else
	{
		CMD_C_OperateOut op;
		op.wOperateCode = WIK_NULL_MY_TIMEOUT;
		op.dwUserID = m_pGameDeskPtr->GetUserID(dwParam);
		OnMsg_PlayerOperate_ByOutCard(dwParam, &op, sizeof(op));
	}
}

CT_VOID CGameProcess::OnTimer_AutoReady()
{
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		m_pGameDeskPtr->SetUserReady(it->first);
	}
}

CT_VOID CGameProcess::OnTimer_PlayerOutCard()
{
	MapChairInfo::iterator it = m_mapChairInfo.find(m_dwAutoOutCardPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "找不到自动出牌的玩家: "<< m_dwAutoOutCardPlayer;
		return;
	}

	//玩家超时出牌分两种情况：没听牌：摸哪张打哪张     听牌：只能出能出的牌
	if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing == CT_TRUE)
	{
		//玩家听牌后，第一次出牌就超时了。
		if(it->second.cbListenCardCount > 0)
		{
			CT_BYTE cbShengyuMaxCardData = INVALID_CARD_DATA, cbFanMaxCardData = INVALID_CARD_DATA;
			CT_BYTE cbShengyuMaxCardData_0 = INVALID_CARD_DATA, cbFanMaxCardData_0 = INVALID_CARD_DATA;
			CT_DWORD  dwShengyuMax = 0, dwFanMax = 0;
			int nLoop = it->second.cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : it->second.cbListenCardCount;
			for(int i = 0; i < nLoop; i++)
			{
				tagListenCardData *pListenCard = (tagListenCardData*)it->second.cbListenCardData[i];
				if(pListenCard)
				{
					int loop = pListenCard->cbHuCardCount > MAX_INDEX ? MAX_INDEX : pListenCard->cbHuCardCount;
					for(int k = 0; k < loop; k++)
					{
						tagHuCardData *pHuCardData = (tagHuCardData *)(pListenCard->cbHuCardData + k * sizeof(tagHuCardData));
						if(pHuCardData)
						{
							if(pHuCardData->wMultipleCount > dwFanMax)
							{
								if(pHuCardData->cbRemainCount > 0)
								{
									dwFanMax = pHuCardData->wMultipleCount;
									cbFanMaxCardData = pListenCard->cbOutCard;
								}
								else
								{
									cbFanMaxCardData_0 = pListenCard->cbOutCard;
								}
							}
							if(pHuCardData->cbRemainCount > dwShengyuMax)
							{
								if(pHuCardData->cbRemainCount > 0)
								{
									dwShengyuMax = pHuCardData->cbRemainCount;
									cbShengyuMaxCardData = pListenCard->cbOutCard;
								}
								else
								{
									cbShengyuMaxCardData_0 = pListenCard->cbOutCard;
								}
							}
						}
					}
				}
			}

			if(cbFanMaxCardData != INVALID_CARD_DATA)
				PlayerOutCard(m_dwAutoOutCardPlayer, cbFanMaxCardData, 0,CT_TRUE);
			else if(cbShengyuMaxCardData != INVALID_CARD_DATA)
				PlayerOutCard(m_dwAutoOutCardPlayer, cbShengyuMaxCardData, 0, CT_TRUE);
			else
			{
				if(cbFanMaxCardData_0 != INVALID_CARD_DATA)
					PlayerOutCard(m_dwAutoOutCardPlayer, cbFanMaxCardData_0, 0,CT_TRUE);
				else if(cbShengyuMaxCardData_0 != INVALID_CARD_DATA)
					PlayerOutCard(m_dwAutoOutCardPlayer, cbShengyuMaxCardData_0, 0, CT_TRUE);
				else
				{
				    CT_BOOL bOut = CT_FALSE;
					//选择一张最大的出牌，防止牌局逻辑卡死
					CT_BYTE cbCardData = INVALID_CARD_DATA;
					for(int m = MAX_INDEX - 1; m >= 0; m--)
					{
						CT_BYTE cbCardDataIndex = it->second.cbHandCardIndex[m];
						if (cbCardDataIndex > 0)
						{
							cbCardData = m_GameLogic.SwitchToCardData(m);
							PlayerOutCard(m_dwAutoOutCardPlayer, cbCardData, 0, CT_TRUE);
							bOut = CT_TRUE;
							break;
						}
					}

					if(bOut == CT_FALSE)
                    {
                        LOG(ERROR) << "玩家："<<it->second.dwUserID << " 全部为0";
                    }
				}
			}
		}
		else
		{
			LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << " 已经听牌，第一出牌超时了，但是查找超时自动出牌找不到牌";
		}
	}
/*	else if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing == CT_FALSE)
	{
		LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << " 已经听牌,并且bIsOutFirstCard_AfterTing为FALS，代码逻辑不应该到这里.";
	}*/
	else
	{
        if(m_GameLogic.IsValidCard(m_dWAutoOutCardData) == CT_FALSE)
        {
            LOG(ERROR) << "自动出的牌: " << (CT_INT32)m_dWAutoOutCardData << " 无效";
            return;
        }

        if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, m_dWAutoOutCardData) == CT_FALSE)
        {
            LOG(ERROR) << "自动出牌的时候, 玩家: "<< it->second.dwUserID << " 没有牌: "<< (CT_INT32)m_dWAutoOutCardData;
            return;
        }

        if(m_bCurOutCardPlayerFinish)
        {
            LOG(ERROR) << "超时，玩家:" << m_pGameDeskPtr->GetUserID(m_dwAutoOutCardPlayer) << " 自动出牌: "<<(int)m_dWAutoOutCardData << " 但是玩家在之前已经完成了出牌，出牌失败";
            return;
        }

		PlayerOutCard(m_dwAutoOutCardPlayer, m_dWAutoOutCardData, 0, CT_TRUE);
	}
}

//用户离开
CT_VOID CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Start)
	{
		//
		return;
	}
	else
	{
		MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
		if(it != m_mapChairInfo.end())
		{
			m_mapChairInfo.erase(it);
		}

		if(m_mapChairInfo.size() == 0)
		{
			m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
			m_pGameDeskPtr->KillGameTimer(IDI_AUTO_JIESAN_DESK);
		}
	}
}

CT_VOID CGameProcess::OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{

}

void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if(m_mapChairInfo.size() >= GAME_PLAYER)
	{
		return;
	}

	MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
	if(it != m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家: "<<it->second.dwUserID << " 调用OnUserEnter 但是还能在房间中找到他";
		return;
	}

	tagChairInfo chair;
	chair.dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
	chair.llCurSocre = m_pGameDeskPtr->GetUserScore(dwChairID);
	chair.llInitScore = m_pGameDeskPtr->GetUserScore(dwChairID);
	chair.llTotalTax = 0;
	memset(chair.cbHandCardIndex, 0, sizeof(chair.cbHandCardIndex));
	m_mapChairInfo[dwChairID] = chair;
}

CT_VOID CGameProcess::DisReConnect(CT_DWORD dwChairID)
{
    MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
    if(it == m_mapChairInfo.end())
	{
    	return;
	}

    CMD_S_StatusPlay data;
    data.wBankerUser = m_dwBanker;
    data.wCurrentUser = m_dwCurOperatePlayer;
    data.wCurOutUser = m_dwCurOutCardPlayer;
	data.wRemainNum = m_cbLeftCardCount;
	data.bCurOutCard = m_cbCurOutCardData;
	if(m_dwCurOperatePlayer == dwChairID)
    {
        data.bCurSendCard = m_cbLastZhuaCardData;
    }
    else
    {
        data.bCurSendCard = INVALID_CARD_DATA;
    }
	if(m_cbCurState == STATE_WAIT_OUT)
	{
		data.cbTotalTimes = TIMER_OUT_CARD;
	}
	else if(m_cbCurState == STATE_WAIT_OUT_OPERATE)
	{
		data.cbTotalTimes = TIMER_OPERATE_TIME_OUT;
	}
	else if(m_cbCurState == STATE_WAIT_SEND_OPERATE)
	{
		data.cbTotalTimes = TIMER_OPERATE_TIME_OUT_AFTER_ZHUACARD;
	}
	else if(m_cbCurState == STATE_JIABEI)
	{
		data.cbTotalTimes = TIMER_AUTO_HU_CARD_AFTER_JIABEI;
	}
	else
	{
		data.cbTotalTimes = 0;
	}

    CT_UINT32 nWasteTime = Utility::GetTick() - m_startOperateTime;
	nWasteTime = nWasteTime / 1000;
	if(nWasteTime >= data.cbTotalTimes)
    {
	    data.cbShengYuTimes = 0;
    }
    else
    {
        data.cbShengYuTimes = data.cbTotalTimes - nWasteTime;
    }

	//data.bOperateFlag = CT_FALSE;
	CT_DOUBLE  dCellScore = m_pGameDeskPtr->GetGameCellScore();
	data.cbCellScore = dCellScore / 100;

	m_GameLogic.SwitchToCardData(it->second.cbHandCardIndex, data.cbHandCardDataIndex);
	for(MapChairInfo::iterator allPlayerIt = m_mapChairInfo.begin(); allPlayerIt != m_mapChairInfo.end(); allPlayerIt++)
	{
		CT_WORD  wLoopCnt = 0;
		for(int i = 0; i < MAX_OUT_NUM; i++)
        {
		    if(allPlayerIt->second.cbDiscardCard[i] == INVALID_CARD_DATA)
            {
                continue;
            }

            data.cbCurOutCardData[allPlayerIt->first][wLoopCnt] = allPlayerIt->second.cbDiscardCard[i];
            wLoopCnt++;
        }
        data.cbCurOutCardNum[allPlayerIt->first] = wLoopCnt;

		CT_BYTE cbWeaveCount = 0;
		CT_BYTE cbActionIndex = 0;
		CT_DWORD  dwActionCode = 0;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(allPlayerIt->second.weaveItem[i].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}

			data.cbCurActionCardData[allPlayerIt->first][cbActionIndex].wProvideUser = allPlayerIt->second.weaveItem[i].wProvideUser;
			data.cbCurActionCardData[allPlayerIt->first][cbActionIndex].cbCardId = allPlayerIt->second.weaveItem[i].cbCenterCard;
			dwActionCode = allPlayerIt->second.weaveItem[i].cbWeaveKind;
			if(dwActionCode & WIK_LEFT_MY || dwActionCode & WIK_RIGHT_MY || dwActionCode & WIK_CENTER_MY)
			{
				dwActionCode |= WIK_CHI_MY;
			}
			else if(dwActionCode & WIK_GANG_MING_MY || dwActionCode & WIK_GANG_AN_MY || dwActionCode & WIK_GANG_MING_PENG_MY)
			{
				dwActionCode |= WIK_GANG_MY;
			}
			data.cbCurActionCardData[allPlayerIt->first][cbActionIndex].wOperateCode = dwActionCode;
			cbActionIndex++;
			cbWeaveCount++;
		}
		data.cbCurActionGroupNum[allPlayerIt->first] = cbWeaveCount;
		data.cbCurHandCardDataNum[allPlayerIt->first] = m_GameLogic.GetCardCount(allPlayerIt->second.cbHandCardIndex);

		//LOG(ERROR) << "玩家: "<< m_pGameDeskPtr->GetUserID(allPlayerIt->first) << " 手牌数:" << (int)data.cbCurHandCardDataNum[allPlayerIt->first];

		if(allPlayerIt->second.bIsTing && allPlayerIt->second.bIsOutFirstCard_AfterTing == CT_FALSE)
		{
			data.bIsTing[allPlayerIt->first] = CT_TRUE;
		}
		else
		{
			data.bIsTing[allPlayerIt->first] = CT_FALSE;
		}
		data.cbJiaBei[allPlayerIt->first] = std::pow(2,allPlayerIt->second.wJiaBei);

		if(allPlayerIt->first != dwChairID)
		{
			if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
			{
			    data.cbShow = 1;
				m_GameLogic.SwitchToCardData(allPlayerIt->second.cbHandCardIndex, data.cbOtherPlayerHandCard);
			}
			else
			{
			    data.cbShow = 0;
				memset(data.cbOtherPlayerHandCard, 0, sizeof(CT_BYTE) * MAX_HAND_CARD_COUNT);
			}
		}
	}

	SendTableData(it->first, SUB_S_GAME_RELINK, &data,sizeof(data));
	LOG_IF(WARNING, PRINT_LOG_INFO)<<"给玩家:" << it->second.dwUserID << " 发送场景消息";

	if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
	{
		//两个都听牌才发送胡牌信息给断线重连者
		for(MapChairInfo::iterator itMayByHuCard = m_mapChairInfo.begin(); itMayByHuCard != m_mapChairInfo.end(); itMayByHuCard++)
		{
			if(itMayByHuCard->second.bIsTing && itMayByHuCard->second.bIsOutFirstCard_AfterTing == CT_FALSE)
			{
				CMD_S_HaveHuCard_Ting data;
				data.cbBei = std::pow(2, itMayByHuCard->second.wJiaBei);
				data.wCurrentUser = itMayByHuCard->first;
				memcpy(data.cbHuCard, itMayByHuCard->second.cbHuCard_DisReconect, sizeof(itMayByHuCard->second.cbHuCard_DisReconect));

				for(int s = 0; s < MAX_HU_CARD_COUNT; s++)
				{
					if(data.cbHuCard[s].cbCardCount > 4)
					{
						data.cbHuCard[s].cbCardCount = 0;
					}
				}

				SendTableData(dwChairID, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data));
			}
		}
	}
	else
	{
		//只有一个玩家听牌，并且是断线重连者听牌，就只发送断线重连者的胡牌信息给他
		MapChairInfo::iterator itMayByHuCard = m_mapChairInfo.find(dwChairID);
		if(itMayByHuCard != m_mapChairInfo.end())
		{
			if(itMayByHuCard->second.bIsTing && itMayByHuCard->second.bIsOutFirstCard_AfterTing == CT_FALSE)
			{
				CMD_S_HaveHuCard_Ting data;
				data.cbBei = std::pow(2, itMayByHuCard->second.wJiaBei);
				data.wCurrentUser = itMayByHuCard->first;
				memcpy(data.cbHuCard, itMayByHuCard->second.cbHuCard_DisReconect, sizeof(itMayByHuCard->second.cbHuCard_DisReconect));

				for(int s = 0; s < MAX_HU_CARD_COUNT; s++)
				{
					if(data.cbHuCard[s].cbCardCount > 4)
					{
						data.cbHuCard[s].cbCardCount = 0;
					}
				}

				SendTableData(dwChairID, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data));
			}
		}
	}

	if(m_cbCurState == STATE_WAIT_OUT)
	{
		//发送出牌提示
		if(m_dwShouldOutCardPlayer == it->first)
		{
			CMD_S_OutTip outTip;
			outTip.wOutCardUser = it->first;
			outTip.wUpDateTime = data.cbShengYuTimes;
			outTip.bZhuangFirstOutCard = m_bZhuangFirstOutCard;
			SendTableData(it->first,SUB_S_OUT_TIP, &outTip, sizeof(outTip));
		}

		//已经听牌但是没有出牌
		if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing)
		{
			//听牌后没出牌就发114 101 115
			CMD_S_OutTip_Ting ting;
			CT_WORD  wLoop = it->second.cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : it->second.cbListenCardCount;
			for(int i = 0; i < wLoop; i++)
			{
				tagListenCardData *pListen = (tagListenCardData *)it->second.cbListenCardData[i];
				if(pListen && pListen->cbOutCard != INVALID_CARD_DATA)
				{
					ting.bOutCardData[i] = pListen->cbOutCard;
				}
			}
			SendTableData(it->first, SUB_S_OUT_TIP_TING, &ting, sizeof(ting));
			LOG_IF(WARNING, PRINT_LOG_INFO) << "给玩家: " << it->second.dwUserID << " 发送114";

			CMD_S_OutTip outTip;
			outTip.wOutCardUser = it->first;
			outTip.wUpDateTime = data.cbShengYuTimes;
			outTip.bZhuangFirstOutCard = m_bZhuangFirstOutCard;
			SendTableData(INVALID_CHAIR,SUB_S_OUT_TIP, &outTip, sizeof(outTip));

			TipPlayerMax(it->first);
		}

		//能听牌但是没有点击听牌
        if(it->second.bIsTing == CT_FALSE && it->second.cbPlayerAction & WIK_TING_MY)
        {
            CMD_S_OperateTip_Send tip;
            tip.wOperateCode = it->second.cbPlayerAction;
            SendTableData(it->first, SUB_S_OPERATE_TIP_SEND, &tip, sizeof(tip));
            TipPlayerMax(it->first);
        }
	}
	else if(m_cbCurState == STATE_WAIT_OUT_OPERATE)
	{
	    if(it->first == m_dwCurOperatePlayer)
        {
            CMD_S_OperateTip tip;
            tip.cbCardId = it->second.cbPlayerActionCardData;
            tip.wOperateCode = it->second.cbPlayerAction;
            SendTableData(it->first, SUB_S_OPERATE_TIP_OUT, &tip, sizeof(tip));
        }
	}
	else if(m_cbCurState == STATE_WAIT_SEND_OPERATE)
	{
		if(it->first == m_dwCurOperatePlayer)
		{
			CMD_S_OperateTip_Send tip;
			tip.wOperateCode = it->second.cbPlayerAction;
			if(it->second.cbPlayerAction & WIK_HU_MY)
			{
				tip.bGangCount = 0;
			}
			if(it->second.cbPlayerAction & WIK_GANG_MY)
			{
				tip.bGangCount = it->second.canGangCard.cbCardCount;
				int nLoop = tip.bGangCount > GANG_CARD_MAX ? GANG_CARD_MAX : tip.bGangCount;
				for(int i = 0; i < nLoop; i++)
				{
					tip.cbCardTB[i].cbCardId = it->second.canGangCard.cbCardData[i];
					tip.cbCardTB[i].wGangOperateCode = it->second.canGangCard.dwGangCardType[i];
				}
			}
			SendTableData(it->first, SUB_S_OPERATE_TIP_SEND, &tip, sizeof(tip));

			//还要提示出牌，而且不能启动出牌超时定时器
            CMD_S_OutTip outTip;
            outTip.wOutCardUser = it->first;
            outTip.wUpDateTime = data.cbShengYuTimes;
			outTip.bZhuangFirstOutCard = m_bZhuangFirstOutCard;
            SendTableData(it->first,SUB_S_OUT_TIP, &outTip, sizeof(outTip));
		}

		if(it->second.bIsTing == CT_FALSE && it->second.cbPlayerAction & WIK_TING_MY)
		{
            CMD_S_OutTip outTip;
            outTip.wOutCardUser = it->first;
            outTip.wUpDateTime = data.cbShengYuTimes;
			outTip.bZhuangFirstOutCard = m_bZhuangFirstOutCard;
            SendTableData(it->first,SUB_S_OUT_TIP, &outTip, sizeof(outTip));

			TipPlayerMax(it->first);
		}
	}
	else if(m_cbCurState == STATE_JIABEI)
	{
	    if(m_dwCurOperatePlayer == it->first)
        {
            CMD_S_OperateTip_Ting ting;
            ting.wOperateCode = it->second.cbPlayerAction;
            SendTableData(it->first, SUB_S_OPERATE_TING, &ting, sizeof(ting));
        }
	}
	else
	{
		//LOG(ERROR) << "玩家: "<< it->second.dwUserID << " 断线重连没有行为";
	}
}

CT_VOID CGameProcess::CfgPlayerHandCard(CT_DWORD dwPlayer)
{
	//配牌要固定庄家，庄家固定为0  闲家固定为1
    CT_BYTE cbCfgHandCard[14];
    cbCfgHandCard[0] = 0x01;
    cbCfgHandCard[1] = 0x02;
    cbCfgHandCard[2] = 0x02;
    cbCfgHandCard[3] = 0x03;
    cbCfgHandCard[4] = 0x03;
    cbCfgHandCard[5] = 0x03;
    cbCfgHandCard[6] = 0x04;
    cbCfgHandCard[7] = 0x06;
    cbCfgHandCard[8] = 0x07;
    cbCfgHandCard[9] = 0x08;
    cbCfgHandCard[10] = 0x09;
    cbCfgHandCard[11] = 0x09;
    cbCfgHandCard[12] = 0x09;
	cbCfgHandCard[13] = 0x05;

	CT_BYTE cbCfgXianCard[13];
	cbCfgXianCard[0] = 0x01;
	cbCfgXianCard[1] = 0x01;
	cbCfgXianCard[2] = 0x04;
	cbCfgXianCard[3] = 0x04;
	cbCfgXianCard[4] = 0x04;
	cbCfgXianCard[5] = 0x07;
	cbCfgXianCard[6] = 0x07;
	cbCfgXianCard[7] = 0x08;
	cbCfgXianCard[8] = 0x08;
	cbCfgXianCard[9] = 0x09;
	cbCfgXianCard[10] = 0x31;
	cbCfgXianCard[11] = 0x31;
	cbCfgXianCard[12] = 0x31;


    //配置庄家
	if(dwPlayer == m_dwBanker)
    {
	    for(int i = 0 ; i < 14; i++)
        {
            if( cbCfgHandCard[i] == m_cbRepertoryCard[i])
            {
                continue;
            }

            for(int j = i+1/*27*/; j < MAX_REPERTORY;j++)
            {
                if(cbCfgHandCard[i] == m_cbRepertoryCard[j])
                {
                    CT_BYTE cbTemp = m_cbRepertoryCard[i];
                    m_cbRepertoryCard[i] = m_cbRepertoryCard[j];
                    m_cbRepertoryCard[j] = cbTemp;
					break;
                }
            }
        }

        m_cbRepertoryCard[0] = 0x09;
        m_cbRepertoryCard[1] = 0x09;
        m_cbRepertoryCard[2] = 0x01;
        m_cbRepertoryCard[3] = 0x02;
        m_cbRepertoryCard[4] = 0x03;
        m_cbRepertoryCard[5] = 0x07;
        m_cbRepertoryCard[6] = 0x08;
        m_cbRepertoryCard[7] = 0x09;
        m_cbRepertoryCard[8] = 0x04;
        m_cbRepertoryCard[9] = 0x06;
        m_cbRepertoryCard[10] = 0x05;
        m_cbRepertoryCard[11] = 0x32;
        m_cbRepertoryCard[12] = 0x32;
        m_cbRepertoryCard[13] = 0x32;
    }
    //配置闲家
    else
    {
        for(int i = 14; i < 27; i++)
		{
        	if(cbCfgXianCard[i-14] == m_cbRepertoryCard[i])
			{
				continue;
			}

			for(int j = 27/*0*/; j < MAX_REPERTORY;j++)
			{
/*				if(j>=14 && j<i)
				{
					continue;
				}*/

				if(cbCfgXianCard[i-14] == m_cbRepertoryCard[j])
				{
					CT_BYTE cbTemp = m_cbRepertoryCard[i];
                    m_cbRepertoryCard[i] = m_cbRepertoryCard[j];
					m_cbRepertoryCard[j] = cbTemp;
					break;
				}

			}
		}

		m_cbRepertoryCard[14] = 0x31;
		m_cbRepertoryCard[15] = 0x02;
		m_cbRepertoryCard[16] = 0x03;
		m_cbRepertoryCard[17] = 0x04;
		m_cbRepertoryCard[18] = 0x05;
		m_cbRepertoryCard[19] = 0x06;
		m_cbRepertoryCard[20] = 0x07;
		m_cbRepertoryCard[21] = 0x08;
		m_cbRepertoryCard[22] = 0x09;
		m_cbRepertoryCard[23] = 0x07;
		m_cbRepertoryCard[24] = 0x07;
		m_cbRepertoryCard[25] = 0x31;
		m_cbRepertoryCard[26] = 0x32;
	}


   m_cbRepertoryCard[27] = 0x32;
    m_cbRepertoryCard[28] = 0x32;
    m_cbRepertoryCard[29] = 0x32;
    m_cbRepertoryCard[30] = 0x32;
    m_cbRepertoryCard[31] = 0x32;
    m_cbRepertoryCard[32] = 0x32;
    m_cbRepertoryCard[33] = 0x09;
    m_cbRepertoryCard[34] = 0x36;
    m_cbRepertoryCard[63] = 0x07;

}

//游戏开始
CT_VOID CGameProcess::OnEventGameStart()
{
    if (m_pGameDeskPtr)
    {
        m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
        if (m_PrivateTableInfo.dwRoomNum != 0)
            m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER);
    }

    //游戏记录的空闲场景构造
    CMD_S_StatusFree StatusFree;
    memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
    StatusFree.dCellScore = m_pGameDeskPtr->GetGameCellScore() * TO_DOUBLE;
    m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SUB_S_GAMESCENE_FREE);

    m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
	//每一局开始重置桌子数据
	ResetTableData_PerGameRound();

	//得到牌堆
	m_GameLogic.RandCardData(m_cbRepertoryCard, MAX_REPERTORY);
	CT_BYTE tempCardCount = 0;
	for(int i = 0 ; i < MAX_REPERTORY; i++)
	{
		if(m_cbRepertoryCard[i] != INVALID_CARD_DATA)
		{
			tempCardCount++;
		}
	}
	m_cbLeftCardCount = tempCardCount;

	//确定庄家
	//NiqiuSrand();
	if (m_dwBanker == INVALID_CHAIR_ID || m_dwWinerChaifID == INVALID_CHAIR_ID || m_dwWinerChaifID == INVALID_CHAIR)
	{
		m_dwBanker = rand() % GAME_PLAYER;
	}
	else
	{
		m_dwBanker = m_dwWinerChaifID;
	}

   /* {
        //在抓手牌前配置手牌
        MapChairInfo::iterator it = m_mapChairInfo.begin();
        m_dwBanker = it->first;
		CfgPlayerHandCard(m_dwBanker);
		if(m_dwBanker == 1)
		{
			CfgPlayerHandCard(0);
		}
		else
		{
			CfgPlayerHandCard(1);
		}
    }*/

	//每个玩家都抓手牌
	PlayerFetchHandCards();

	//发牌
	SendCard_GameStart();
    LOG_IF(WARNING, PRINT_LOG_INFO)  << "开始发牌";

    //游戏逻辑开始，设置监控定时器
    m_pGameDeskPtr->SetGameTimer(IDI_AUTO_JIESAN_DESK, TIMER_AUTO_JIESAN_DESK);

	m_dwCurOperatePlayer = m_dwBanker;
	m_pGameDeskPtr->SetGameTimer(IDI_START_GAME, TIMER_START_GAME * 1000);
}

CT_VOID CGameProcess::SendCard_GameStart()
{
	CMD_S_GameStart 		gameStart;
	CT_DOUBLE dTemp;
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		memset(&gameStart, 0, sizeof(gameStart));
		gameStart.wBankerUser = m_dwBanker;
		gameStart.wCurrentUser = it->first;
		gameStart.wRemainNum = m_cbLeftCardCount;
		dTemp = m_pGameDeskPtr->GetGameCellScore();
		gameStart.lCellScore = dTemp / 100;
		gameStart.wUpDateTime = TIME_OUT;
		size_t initHandCardCount = it->second.cbInitHandCardData.size();
		size_t wLoopCnt = initHandCardCount > DEALER_CARD_COUNT ? DEALER_CARD_COUNT : initHandCardCount;
		for(size_t i = 0; i < wLoopCnt; i++)
		{
			gameStart.cbHandCardDataIndex[i] = it->second.cbInitHandCardData[i];
		}

		SendTableData(it->first, SUB_S_GAME_START,  &gameStart, sizeof(gameStart), true);
	}
}

CT_VOID CGameProcess::PlayerFetchHandCards()
{
	for (MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		it->second.clearAllHandCard();

		int i;
		for(i = 0; i < NO_DEALER_CARD_COUNT; i++)
		{
			PlayerFetchOneCard(it->first);
		}

		if(it->first == m_dwBanker)
		{
			//庄家多抓一张
			CT_BYTE cbCardData = PlayerFetchOneCard(it->first);
			//庄家最后一张为超时自动打的牌
			m_cbZhuaCardData = cbCardData;
		}
	}
}

CT_BYTE CGameProcess::PlayerFetchOneCard(CT_WORD wPlayerChairID)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(wPlayerChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "通过椅子ID: " << wPlayerChairID << " 查找不到玩家信息";
		return INVALID_CARD_DATA;
	}

	CT_BYTE cbCardData = INVALID_CARD_DATA;
	cbCardData = m_cbRepertoryCard[m_dwCurRepertoryCardIndex];
	assert(cbCardData != INVALID_CARD_DATA);
	CT_WORD wCardIndex = m_GameLogic.SwitchToCardIndex(cbCardData);
	assert(wCardIndex < MAX_INDEX);
	it->second.cbHandCardIndex[wCardIndex]++;
	it->second.cbInitHandCardData.push_back(cbCardData);

	m_cbRepertoryCard[m_dwCurRepertoryCardIndex] = INVALID_CARD_DATA;
	m_dwCurRepertoryCardIndex++;
	m_cbLeftCardCount--;

	if(cbCardData == INVALID_CARD_DATA)
	{
		LOG(ERROR) << "严重错误:玩家ID: "<< it->second.dwUserID << " 抓到无效的牌";
	}

	return cbCardData;
}

CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	if(wChairID >= GAME_PLAYER)
	{
		LOG(ERROR) << "OnGameMessage wChairID error: " << wChairID << " dwSubCmdID:" << dwSubCmdID ;
		return CT_TRUE;
	}
	if(pDataBuffer == NULL)
	{
		LOG(ERROR) << "OnGameMessage pDataBuffer is nullptr" << " dwSubCmdID:" << dwSubCmdID;
		return CT_TRUE;
	}

	switch(dwSubCmdID)
	{
		case SUB_C_OUT_CARD:
			{
				return OnMsg_PlayerOutCard(wChairID, pDataBuffer, dwDataSize);
				break;
			}
		case SUB_C_OPERATE_OUT:
			{
				return OnMsg_PlayerOperate_ByOutCard(wChairID, pDataBuffer, dwDataSize);
				break;
			}
	    case SUB_C_OPERATE_SEND:
            {
                return OnMsg_PlayerOperate_ByZhuaCard(wChairID, pDataBuffer, dwDataSize);
                break;
            }
		case SUB_C_READY_OUT_CARD:
			{
				return OnMsg_ReadyOutCard(wChairID, pDataBuffer, dwDataSize);
				break;
			}
		case SUB_C_OPERATE_TING:
			{
				return OnMsg_PlayerJiaBei(wChairID, pDataBuffer, dwDataSize);
				break;
			}
	    case SUB_C_OPERATE_TING_CANCEL:
            {
                return OnMsg_PlayerCancenTing(wChairID, pDataBuffer, dwDataSize);
                break;
            }
		default:
			LOG(ERROR) << "收到玩家: " << m_pGameDeskPtr->GetUserID(wChairID) << "发来的消息:" <<dwSubCmdID;
			break;
	}

    return CT_TRUE;
}

CT_BOOL CGameProcess::OnMsg_PlayerCancenTing(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize)
{
	if(dwDataSize != sizeof(CMD_C_OperateTing_Cancel))
	{
		LOG(ERROR) << "玩家: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 执行取消听牌操作，dwDataSize:"<< dwDataSize
		<< " sizeof(CMD_C_OperateTing_Cancel):" << sizeof(CMD_C_OperateTing_Cancel);
		return CT_TRUE;
	}

	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "执行取消听牌操作时，通过椅子ID: "<< wChairID << " 找不到玩家";
		return CT_TRUE;
	}

	if(it->second.bIsTing == CT_FALSE)
	{
		LOG(ERROR) << "玩家: "<< it->second.dwUserID << " 没有听牌，但是却发送了取消听牌消息";
		return CT_TRUE;
	}

	if(m_dwCurOperatePlayer != wChairID)
	{
		LOG(ERROR) << "玩家: "<< it->second.dwUserID << "执行了取消听牌操作,但是当前的操作者应该是: " << m_pGameDeskPtr->GetUserID(m_dwCurOperatePlayer);
		return CT_TRUE;
	}

	if(m_cbCurState != STATE_WAIT_SEND_OPERATE && m_cbCurState != STATE_WAIT_OUT)
	{
		LOG(ERROR) << "玩家: "<<it->second.dwUserID << "执行取消听牌操作，但是目前的操作状态应该是: "<<(int)m_cbCurState;
		return CT_TRUE;
	}

	LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 取消听牌";

	//玩家取消听牌就清除玩家的行为
	it->second.clearPlayerAction();

	//玩家取消听牌
	it->second.bIsTing = CT_FALSE;

	CMD_S_Cancel_Ting ting;
	ting.wOperateCode = 0;
	SendTableData(it->first, SUB_S_CANCEL_TING, &ting, sizeof(ting), true);

	//玩家取消了听牌继续提示玩家听牌
	if(CanTing(wChairID))
	{
		m_eTingReason = TingTrigger::CancelTing;
		//为玩家不听牌，超时出牌做准备
		m_dwAutoOutCardPlayer = wChairID;
		m_dWAutoOutCardData = m_cbCancelTing_OutTime_OutCard;
		TipPlayerAction(wChairID, m_cbCanTingTrigger_ActionCardData, m_eCanTingTrigger, CT_FALSE);
	}

	return CT_TRUE;
}

CT_BOOL CGameProcess::OnMsg_PlayerJiaBei(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize)
{
	if(m_cbCurState != STATE_JIABEI)
	{
		LOG(ERROR) << "玩家: "<< m_pGameDeskPtr->GetUserID(wChairID) << " 执行听后操作，但是牌局状态是: "<<(int)m_cbCurState;
		return CT_TRUE;
	}

	if(dwDataSize != sizeof(CMD_C_OperateTing))
	{
		LOG(ERROR) << "dwDataSize: "<< dwDataSize << " is not equal to sizeof(CMD_C_OperateTing)" << sizeof(CMD_C_OperateTing);
		return CT_TRUE;
	}

	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "OnMsg_PlayerJiaBei 通过椅子ID: "<<wChairID << " 找不到玩家";
		return CT_TRUE;
	}

	if(m_dwCurOperatePlayer != wChairID)
	{
		LOG(ERROR) << "玩家: "<<it->second.dwUserID << " 执行听后胡加倍操作,但是目前应该是玩家:" << m_pGameDeskPtr->GetUserID(m_dwCurOperatePlayer) << " 来执行这项操作";
		return CT_TRUE;
	}

	CMD_C_OperateTing *pData = (CMD_C_OperateTing *)pDataBuffer;
	if(!(pData->wOperateCode & WIK_JIABEI) && !(pData->wOperateCode & WIK_HU_MY))
    {
	    //此处玩家只能加倍 胡
	    LOG(ERROR) << "OnMsg_PlayerJiaBei 玩家在这里只能胡和加倍,但是收到的操作是:"<<pData->wOperateCode;
	    return CT_TRUE;
    }

	//是否能加倍
	if(pData->wOperateCode & WIK_JIABEI)
	{
		if(!(it->second.cbPlayerAction & WIK_JIABEI))
		{
			LOG(ERROR) << "玩家: "<<it->second.dwUserID << " 不能加倍，但是却发送了加倍消息给服务器";
			return CT_TRUE;
		}
	}

	//是否能胡
	if(pData->wOperateCode & WIK_HU_MY)
	{
		if(!(it->second.cbPlayerAction & WIK_HU_MY))
		{
			LOG(ERROR) << "玩家: "<< it->second.dwUserID << "不能胡牌，但是却发送了加倍后的胡牌消息给服务器";
			return CT_TRUE;
		}

		if(!(pData->wOperateCode & it->second.cbPlayerAction))
        {
		    LOG(ERROR) <<"玩家: "<< it->second.dwUserID << " 玩家发的胡牌类型是: " << pData->wOperateCode << " 玩家能胡的类型是: " << it->second.cbPlayerAction;
		    return CT_TRUE;
        }
	}

    CT_BOOL bZhuaCard = CT_TRUE;

    if(it->second.cbPlayerAction & WIK_HU_ZI_MO_MY)
    {
        bZhuaCard = CT_TRUE;
    }
    else if(it->second.cbPlayerAction & WIK_HU_FANG_PAO_MY)
    {
        bZhuaCard = CT_FALSE;
    }
    else
    {
        LOG(ERROR) << "判断抓牌还是出牌引发的加倍失败";
        return CT_TRUE;
    }

	CT_BOOL bZimo = CT_FALSE;
	if(pData->wOperateCode & WIK_HU_MY)
	{
		if(pData->wOperateCode & WIK_HU_ZI_MO_MY)
		{
			bZimo = CT_TRUE;
		}
		else if(pData->wOperateCode & WIK_HU_FANG_PAO_MY)
		{
			bZimo = CT_FALSE;
		}
		else
		{
			LOG(ERROR) << "玩家: "<<it->second.dwUserID << " 听后胡牌，发送的胡牌但是没有加入自摸胡或者放炮胡";
		}
	}

	CT_BOOL bSucc = CT_FALSE;
	//这个时候只能 加倍 胡
	if(pData->wOperateCode & WIK_JIABEI)
	{
		bSucc = PlayerJiaBei(wChairID);
		if(bSucc)
		{
			//是否是自己抓牌引发加倍
			CT_BOOL bSelfZhuaJiaBei = CT_FALSE;
			if(it->second.dwActionProvider == wChairID)
			{
				bSelfZhuaJiaBei = CT_TRUE;
			}
			else
			{
				bSelfZhuaJiaBei = CT_FALSE;
			}

			if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
			{
				TipPlayerMayBeHuCard_AfterTing(wChairID, it->second.cbPlayerActionCardData, bSelfZhuaJiaBei, CT_FALSE);
			}
			else
			{
				TipPlayerMayBeHuCard_AfterTing(wChairID, it->second.cbPlayerActionCardData, bSelfZhuaJiaBei, CT_FALSE, CT_FALSE);
			}
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家 :" << it->second.dwUserID << "  成功加倍 ";
		}
	}
	else if(pData->wOperateCode & WIK_HU_MY)
	{
		//超时也用胡牌处理
		bSucc = PlayerChiHuCard(wChairID, it->second.cbPlayerActionCardData, bZimo);
		if(bSucc)
			LOG_IF(WARNING, PRINT_LOG_INFO) <<"玩家: "<<it->second.dwUserID << " 听后胡牌";
	}
	else
	{
		bSucc = CT_FALSE;
		LOG(ERROR) << "玩家: "<< it->second.dwUserID << " 在发送加倍消息205的时候，发送给服务器的操作命令:"<<pData->wOperateCode << " 错误";
	}

	if(bSucc)
	{
        m_cbCurState = STATE_DEFAUT;

		//清除玩家行为数据之前获取是否能抢杠胡
		CT_BOOL bCanQiangGangHu = it->second.bCanQiangGangHu;

		//服务器超时过，给客户端发送取消提示消息
		CMD_S_OperateTip_Cancel cancel;
		cancel.wOperateCode = WIK_NULL_MY;
		SendTableData(wChairID, SUB_S_OPERATE_TIP_CANCEL, &cancel, sizeof(cancel), true);

		if(pData->wOperateCode & WIK_HU_MY)
		{
			if(bZimo)
            {
				CMD_S_OperateSend tip;
				tip.wOperateCode = 0;
				tip.wOperateCode |= WIK_HU_MY;
				tip.wOperateCode |= WIK_HU_ZI_MO_MY;
				tip.wOperateUser = wChairID;
				tip.cbCardId = it->second.cbPlayerActionCardData;

                for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
                {
                    if(p == m_dwBanker)
                    {
                        SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), true);
                    }
                    else
                    {
                        SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), false);
                    }
                }
            }
            else
            {
				CMD_S_OperateOut tip;
				tip.wOperateCode = 0;
				tip.wOperateCode |= WIK_HU_MY;
				tip.wOperateCode |= WIK_HU_FANG_PAO_MY;
				tip.cbCardId = it->second.cbPlayerActionCardData;
				if(bCanQiangGangHu)
					tip.isQiangGang = CT_TRUE;
				else
					tip.isQiangGang = CT_FALSE;
				tip.wOperateUser = wChairID;
				tip.wProvideUser = it->second.dwActionProvider;

                for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
                {
                    if(p == m_dwBanker)
                    {
                        SendTableData(p, SUB_S_OPERATE_OUT, &tip, sizeof(tip), true);
                    }
                    else
                    {
                        SendTableData(p, SUB_S_OPERATE_OUT, &tip, sizeof(tip), false);
                    }
                }
            }

		}

		SetMaxActionData(wChairID, pData->wOperateCode);

		//玩家操作完成,清除玩家的行为数据
		it->second.clearPlayerAction();

		//所有操作完成进行下一步
		/*CT_BOOL bIsHave = CT_FALSE;
		for (MapChairInfo::iterator itAll = m_mapChairInfo.begin(); itAll != m_mapChairInfo.end(); itAll++)
		{
			if (itAll->second.cbPlayerAction != WIK_NULL_MY)
			{
				bIsHave = CT_TRUE;
				break;
			}
		}*/

		//抓牌后引发的行为,所有玩家都响应了
		m_pGameDeskPtr->KillGameTimer(IDI_AUTO_HU_CARD_AFTER_JIABEI);

		if(m_dwMaxAction == WIK_JIABEI)
		{
			if(bCanQiangGangHu)
			{
				//听后胡牌提示，在能抢杠胡的情况下点击了加倍，就让巴杠玩家抓牌
				PlayerZhuaCard(Action_GangCard,m_dwCurGangPlayer);
			}
			else
			{
				//否则就走以前的流程
				if(bZhuaCard)
				{
					//自己抓拍引发加倍，下一步打牌
					m_dwAutoOutCardPlayer = wChairID;
					m_dWAutoOutCardData = m_cbZhuaCardData;
					m_dwCurOperatePlayer = wChairID;
					m_cbCurState = STATE_WAIT_OUT;
					m_pGameDeskPtr->SetGameTimer(IDI_AUTO_OUT_CARD_AFTER_TING, TIMER_AUTO_OUT_CARD_AFTER_TING);
				}
				else
				{
					//别人打牌引发加倍,下一步自己抓牌
					PlayerZhuaCard(Action_JiaBei, wChairID);
				}
			}
		}
		else
		{
			PlayerFinishAction(m_dwMaxActionPlayer, m_dwMaxAction);
		}
	}

	return CT_TRUE;
}

CT_BOOL CGameProcess::OnMsg_ReadyOutCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize)
{
    if(dwDataSize != sizeof(CMD_C_Ready_OutCard))
    {
    	LOG(ERROR) << "dwDataSize :" << dwDataSize << " is not equal to :"<< sizeof(CMD_C_Ready_OutCard);
    	return CT_TRUE;
    }

    MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
    if(it == m_mapChairInfo.end())
	{
    	LOG(ERROR) << "OnMsg_ReadyOutCard  通过椅子ID:"<<wChairID << " 找不到玩家";
    	return CT_TRUE;
	}

	CT_BYTE cbHandCardCnt = m_GameLogic.GetCardCount(it->second.cbHandCardIndex);
    if(cbHandCardCnt != 14 && cbHandCardCnt != 11 && cbHandCardCnt != 8 && cbHandCardCnt != 5 && cbHandCardCnt != 2)
	{
		CMD_S_HaveHuCard data;
		data.cbHuCount = 0;
		memset(&data.cbHuCard, 0 , sizeof(data.cbHuCard));
		SendTableData(wChairID, SUB_S_MABY_HU_CARD, &data, sizeof(data), false);

    	//LOG(ERROR) << "玩家ID: "<<it->second.dwUserID << " 发送 ReadyOutCard消息的时候,手牌数量: "<< (int)cbHandCardCnt << " 错误";
    	return CT_TRUE;
	}

	CMD_C_Ready_OutCard *pData = (CMD_C_Ready_OutCard *)pDataBuffer;
    CT_BYTE cbCardDataCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, pData->cbCardId);
    if(cbCardDataCnt == 0)
	{
    	LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << "牌: "<<(int)pData->cbCardId << " 数量为0";
    	return CT_TRUE;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//获取能胡牌的具体数据
	//所有玩家手牌信息
	CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
	memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
	//CT_BYTE cbCurPlayerIndex = 0;
	//所有玩家弃牌信息
	CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
	memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
	CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
	memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
	//所有玩家的组合项
	tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
	for(int m = 0 ; m < GAME_PLAYER; m++)
	{
		for(int n = 0; n < MAX_WEAVE; n++)
		{
			cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
		}
	}
	CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
	memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

	//所有玩家的听牌状态
	CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
	memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

	//所有玩家是否天听
	CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
	memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

	for(MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
	{
		//获取所有玩家手牌信息
		for(int k = 0 ; k < MAX_INDEX; k++)
		{
			cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
		}

		//获取所有玩家弃牌信息
		for(int k = 0; k < itChair->second.cbDiscardCount; k++)
		{
			cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
		}
		cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

		//获取所有玩家的听牌状态
        cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

		//获取所有玩家的组合项
		CT_BYTE cbWeaveItemCount = 0;
		for(int k = 0; k < MAX_WEAVE; k++)
		{
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}
			memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData, sizeof(itChair->second.weaveItem[k].cbCardData));
			cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
			}
			cbWeaveItemCount++;
		}
		cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
	}
	CT_CHAR szGameRoomName[LEN_SERVER] = "test";
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CT_BYTE cbListenCardData[MAX_LISTEN_CARD_COUNT][2+MAX_INDEX*4];
	memset(cbListenCardData, 0, sizeof(cbListenCardData));
	CT_BYTE cbListenCardCount = 0;
	m_GameLogic.AnalyseTingCard(wChairID, m_dwBanker, cbAllPlayerHandCardIndex, cbAllPlayerDiscardCard, cbAllPlayerDiscardCount, m_cbAllPlayerOutCardCount,
												 cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount, cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0, GetMenFeng(wChairID),
												 m_cbLeftCardCount, m_bGangStatus, szGameRoomName, cbListenCardData, cbListenCardCount);

	if(cbListenCardCount > 0)
	{
		CT_WORD wLoopCnt = cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : cbListenCardCount;
		for(int i = 0 ; i < wLoopCnt; i++)
		{
			tagListenCardData *pListen = (tagListenCardData *)cbListenCardData[i];
			if(pListen && (pListen->cbOutCard == pData->cbCardId))
			{
				//LOG(ERROR) << "========================BEGIN==========================";
				//LOG(ERROR) << "如果出牌: "<<(int)pData->cbCardId;
				CMD_S_HaveHuCard data;
				data.cbHuCount = pListen->cbHuCardCount;
				for(int k = 0 ; k < data.cbHuCount; k++)
				{
					tagHuCardData *pHuCardData = (tagHuCardData *)(pListen->cbHuCardData + (k * sizeof(tagHuCardData)));
					data.cbHuCard[k].cbCardId = pHuCardData->cbListenCard;
					data.cbHuCard[k].cbCardCount = GetShengYuCountAfterTing(wChairID, data.cbHuCard[k].cbCardId);//pHuCardData->cbRemainCount;
					data.cbHuCard[k].cbFanCount = pHuCardData->wMultipleCount;
					//LOG(ERROR) << "能胡: "<< (int) data.cbHuCard[k].cbCardId << " 番数: "<< (int)data.cbHuCard[k].cbFanCount << " 剩余: "<< (int)data.cbHuCard[k].cbCardCount << " 张";
				}

				//LOG(ERROR) << "========================END==========================";
				SendTableData(wChairID, SUB_S_MABY_HU_CARD, &data, sizeof(data), false);
				return CT_TRUE;
			}
		}

        CMD_S_HaveHuCard data;
        data.cbHuCount = 0;
        memset(&data.cbHuCard, 0 , sizeof(data.cbHuCard));
        SendTableData(wChairID, SUB_S_MABY_HU_CARD, &data, sizeof(data), false);
	}
	else
	{
		CMD_S_HaveHuCard data;
		data.cbHuCount = 0;
		memset(&data.cbHuCard, 0 , sizeof(data.cbHuCard));
		SendTableData(wChairID, SUB_S_MABY_HU_CARD, &data, sizeof(data), false);
	}

    return CT_TRUE;
}

CT_BYTE CGameProcess::GetShengYuCountAfterTing(CT_BYTE cbPlayer, CT_BYTE cbCardData)
{

   // LOG_IF(WARNING, PRINT_LOG_INFO) << " BEGING GetShengYuCountAfterTing ";
	CT_BYTE cbShengYu = 4;

	CT_BYTE cbShouPai = 0;
	CT_BYTE cbQiCnt = 0;
	CT_BYTE cbChiCnt = 0;
	CT_BYTE cbGangCnt = 0;
	CT_BYTE cbPengCnt = 0;

	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		if( (it->first == cbPlayer) || (it->second.bIsTing) )
		{
			//从自己的手牌中去掉
			CT_BYTE cbHandCardData[MAX_HAND_CARD_COUNT];
			memset(cbHandCardData, INVALID_CARD_DATA, sizeof(cbHandCardData));
			m_GameLogic.SwitchToCardData(it->second.cbHandCardIndex, cbHandCardData);
			for(int i = 0; i < MAX_HAND_CARD_COUNT; i++)
			{
				if(cbHandCardData[i] == cbCardData)
				{
					if(cbShengYu == 0)
					{
						cbShouPai += 1;
					}

					//LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 手牌中有1张: " << (int)cbCardData;
					//<< " 实际总共有: " << (int)m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCardData) << " 张";
					cbShengYu--;
				}
			}
		}

		//从自己的组合牌中去掉
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if( (it->second.weaveItem[i].cbWeaveKind & WIK_GANG_AN_MY ||
				 it->second.weaveItem[i].cbWeaveKind & WIK_GANG_MING_MY ||
				 it->second.weaveItem[i].cbWeaveKind & WIK_GANG_MING_PENG_MY) &&
				(it->second.weaveItem[i].cbCenterCard == cbCardData))
			{
				if(cbShengYu == 0)
				{
					cbGangCnt += 4;
				}

                //LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 杠牌区中有4张: " << (int)cbCardData;
				cbShengYu -= 4;
			}
			else if(it->second.weaveItem[i].cbWeaveKind & WIK_LEFT_MY ||
					it->second.weaveItem[i].cbWeaveKind & WIK_RIGHT_MY ||
					it->second.weaveItem[i].cbWeaveKind & WIK_CENTER_MY)
			{
				for(int k = 0; k < 4; k++)
				{
					if(it->second.weaveItem[i].cbCardData[k] == cbCardData)
					{
						if(cbShengYu == 0)
						{
							cbChiCnt += 1;
						}

                       // LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 吃牌区中有1张: " << (int)cbCardData;
						cbShengYu -= 1;
						break;
					}
				}
			}
			else if(it->second.weaveItem[i].cbWeaveKind & WIK_PENG_MY &&
					it->second.weaveItem[i].cbCenterCard == cbCardData)
			{
				if(cbShengYu == 0)
				{
					cbPengCnt += 3;
				}

                //LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 碰牌区中有3张: " << (int)cbCardData;
				cbShengYu -= 3;
			}
		}

		//从自己的出牌中去掉
		for(int i = 0; i < MAX_OUT_NUM; i++)
		{
			if(it->second.cbDiscardCard[i] == cbCardData)
			{
				if(cbShengYu == 0)
				{
					cbQiCnt += 1;
				}

                //LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 出牌区中有1张: " << (int)cbCardData;
				cbShengYu--;
			}
		}
	}

	//LOG_IF(WARNING, PRINT_LOG_INFO) << "告诉玩家:"<<m_pGameDeskPtr->GetUserID(cbPlayer) << " 牌:"<< (int)cbCardData << " 剩余:" << (int)cbShengYu;
   // LOG_IF(WARNING, PRINT_LOG_INFO) << " END GetShengYuCountAfterTing ";
	return cbShengYu;
}

CT_VOID CGameProcess::TipPlayerMayBeHuCard_AfterTing(CT_DWORD dwChairID, CT_BYTE cbCardData, CT_BOOL bZiMo, CT_BOOL bFirstOutCard,CT_BOOL bSendTipData /*= CT_TRUE*/)
{
	LOG_IF(WARNING, PRINT_LOG_INFO) << "调用 TipPlayerMayBeHuCard_AfterTing " << (int)cbCardData;
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
	if(it == m_mapChairInfo.end())
	{
		return;
	}

	//CT_BYTE sss = m_GameLogic.GetCardCount(it->second.cbHandCardIndex);
	if(bZiMo == CT_FALSE)
	{
		m_GameLogic.AddCard(it->second.cbHandCardIndex, cbCardData);
	}

	CT_BYTE cbHandCardCnt = m_GameLogic.GetCardCount(it->second.cbHandCardIndex);
	if(cbHandCardCnt != 14 && cbHandCardCnt != 11 && cbHandCardCnt != 8 && cbHandCardCnt != 5 && cbHandCardCnt != 2)
	{
		CMD_S_HaveHuCard_Ting data;
		data.cbBei = std::pow(2,it->second.wJiaBei);
		data.wCurrentUser = dwChairID;
		memset(data.cbHuCard, INVALID_CARD_DATA, sizeof(data.cbHuCard));

		for(int s = 0; s < MAX_HU_CARD_COUNT; s++)
		{
			if(data.cbHuCard[s].cbCardCount > 4)
			{
				data.cbHuCard[s].cbCardCount = 0;
			}
		}

		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == dwChairID)
			{
				SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
			}
			else
			{
			    if(bSendTipData)
                {
                    SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                }
			}
		}

		if(bZiMo == CT_FALSE)
		{
			m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData);
		}
		LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家: " << it->second.dwUserID << " 调用TipPlayerMayBeHuCard_AfterTing的时候，手牌数: "<<(int)cbHandCardCnt;
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//获取能胡牌的具体数据
	//所有玩家手牌信息
	CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
	memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
	//CT_BYTE cbCurPlayerIndex = 0;
	//所有玩家弃牌信息
	CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
	memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
	CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
	memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
	//所有玩家的组合项
	tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
	for(int m = 0 ; m < GAME_PLAYER; m++)
	{
		for(int n = 0; n < MAX_WEAVE; n++)
		{
			cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
		}
	}
	CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
	memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

	//所有玩家的听牌状态
	CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
	memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

	//所有玩家是否天听
	CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
	memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

	for(MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
	{
		//获取所有玩家手牌信息
		for(int k = 0 ; k < MAX_INDEX; k++)
		{
			cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
		}

		//获取所有玩家弃牌信息
		for(int k = 0; k < itChair->second.cbDiscardCount; k++)
		{
			cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
		}
		cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

		//获取所有玩家的听牌状态
		cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

		//获取所有玩家的组合项
		CT_BYTE cbWeaveItemCount = 0;
		for(int k = 0; k < MAX_WEAVE; k++)
		{
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}
			memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData, sizeof(itChair->second.weaveItem[k].cbCardData));
			cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
			}
			cbWeaveItemCount++;
		}
		cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
	}
	CT_CHAR szGameRoomName[LEN_SERVER] = "test";
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CT_BYTE cbListenCardData[MAX_LISTEN_CARD_COUNT][2+MAX_INDEX*4];
	memset(cbListenCardData, 0, sizeof(cbListenCardData));
	CT_BYTE cbListenCardCount = 0;
	m_GameLogic.AnalyseTingCard(dwChairID, m_dwBanker, cbAllPlayerHandCardIndex, cbAllPlayerDiscardCard, cbAllPlayerDiscardCount, m_cbAllPlayerOutCardCount,
								cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount, cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0, GetMenFeng(dwChairID),
								m_cbLeftCardCount, m_bGangStatus, szGameRoomName, cbListenCardData, cbListenCardCount);

	if(bZiMo == CT_FALSE)
	{
		m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData);
	}

	if(cbListenCardCount > 0)
	{
		CT_WORD wLoopCnt = cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : cbListenCardCount;
		for(int i = 0 ; i < wLoopCnt; i++)
		{
			tagListenCardData *pListen = (tagListenCardData *)cbListenCardData[i];
			if(pListen && (pListen->cbOutCard == cbCardData))
			{
				CMD_S_HaveHuCard_Ting data;
				data.cbBei = std::pow(2, it->second.wJiaBei);
				data.wCurrentUser = dwChairID;

				//玩家摊牌 和 加倍的时候通知所有玩家某玩家能胡的牌是什么，番数多少，剩余多少张。这属于普通情况
				memset(it->second.cbHuCard_DisReconect, INVALID_CARD_DATA, sizeof(it->second.cbHuCard_DisReconect));
				for(int k = 0 ; k < pListen->cbHuCardCount; k++)
				{
					tagHuCardData *pHuCardData = (tagHuCardData *)(pListen->cbHuCardData + (k * sizeof(tagHuCardData)));
					data.cbHuCard[k].cbCardId = pHuCardData->cbListenCard;
					data.cbHuCard[k].cbCardCount = GetShengYuCountAfterTing(dwChairID, data.cbHuCard[k].cbCardId);
					data.cbHuCard[k].cbFanCount = pHuCardData->wMultipleCount;
					if(data.cbHuCard[k].cbCardCount > 4)
					{
						data.cbHuCard[k].cbCardCount = 0;
					}

					it->second.cbHuCard_DisReconect[k].cbCardId = data.cbHuCard[k].cbCardId;
					it->second.cbHuCard_DisReconect[k].cbCardCount = data.cbHuCard[k].cbCardCount;
					it->second.cbHuCard_DisReconect[k].cbFanCount = data.cbHuCard[k].cbFanCount;

					if(bFirstOutCard)
					{
						//如果是玩家出牌后玩家就听牌了，那么就把玩家听牌后能胡哪些牌保存起来
						it->second.cbCanHuCard_AfterTing[k] = data.cbHuCard[k].cbCardId;
					}
				}
				for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
				{
					if(p == dwChairID)
					{
                        SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
					}
					else
					{
					    if(bSendTipData)
                        {
                            SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                        }
					}
				}

				//处理特殊情况1:两个玩家听后能胡的某些牌一样,这种情况暂时不考虑
				/*for(MapChairInfo::iterator itMayHu = m_mapChairInfo.begin(); itMayHu != m_mapChairInfo.end(); itMayHu++)
				{
					if(itMayHu->first == dwChairID)
					{
						continue;
					}

					CMD_S_HaveHuCard_Ting dataOther;
					CT_WORD wIndex = 0;
					for(int i = 0 ; i < MAX_HU_CARD_COUNT; i++)
					{
					    if(itMayHu->second.cbCanHuCard_AfterTing[i] == INVALID_CARD_DATA)
                        {
                            continue;
                        }

						for(int j = 0; j < MAX_HU_CARD_COUNT; j++)
						{
							if(itMayHu->second.cbCanHuCard_AfterTing[i] == data.cbHuCard[j].cbCardId)
							{
								dataOther.cbHuCard[wIndex].cbCardId = data.cbHuCard[j].cbCardId;
								dataOther.cbHuCard[wIndex].cbCardCount = data.cbHuCard[j].cbCardCount;
								dataOther.cbHuCard[wIndex].cbFanCount = data.cbHuCard[j].cbFanCount;

								for(int k = 0; k < MAX_HU_CARD_COUNT; k++)
                                {
								    if(itMayHu->second.cbHuCard_DisReconect[k].cbCardId == dataOther.cbHuCard[wIndex].cbCardId)
                                    {
                                        itMayHu->second.cbHuCard_DisReconect[k].cbCardId = dataOther.cbHuCard[wIndex].cbCardId;
                                        itMayHu->second.cbHuCard_DisReconect[k].cbCardCount = dataOther.cbHuCard[wIndex].cbCardCount;
                                        itMayHu->second.cbHuCard_DisReconect[k].cbFanCount = dataOther.cbHuCard[wIndex].cbFanCount;
                                        break;
                                    }
                                }

								wIndex++;
                                break;
							}
						}

                        if(wIndex >= MAX_HU_CARD_COUNT)
                        {
                            break;
                        }
					}

					if(wIndex > 0)
                    {
                        dataOther.wCurrentUser = itMayHu->first;
                        dataOther.cbBei = std::pow(2, itMayHu->second.wJiaBei);
                        SendTableData(itMayHu->first, SUB_S_MABY_HU_CARD_TING, &dataOther, sizeof(dataOther), true);
                    }
				}*/

				//处理特殊情况2:玩家B听牌的时候，玩家A已经听牌，并且玩家B手牌中某些牌是玩家A能胡的牌.这种情况目前只修改张数，不修改翻数。
				if(bFirstOutCard)
                {
				    for(MapChairInfo::iterator itSecond = m_mapChairInfo.begin(); itSecond != m_mapChairInfo.end(); itSecond++)
                    {
				        if(itSecond->first == dwChairID)
                        {
                            continue;
                        }

                        if(itSecond->second.bIsTing == CT_FALSE)
                        {
                            //对面没听牌，所以我的摊牌不会影响对方可胡牌的剩余牌张数
                            break;
                        }

                        CT_BOOL bUpdate = CT_FALSE;
                        CT_BYTE cbCanHuCardData = INVALID_CARD_DATA;
                        CT_BYTE cbCanHuCardCount = 0;
                        for(int i = 0; i < MAX_HU_CARD_COUNT; i++)
						{
                        	if(itSecond->second.cbHuCard_DisReconect[i].cbCardId == INVALID_CARD_DATA)
							{
								continue;
							}

							cbCanHuCardData = itSecond->second.cbHuCard_DisReconect[i].cbCardId;
							cbCanHuCardCount = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCanHuCardData);
							if(cbCanHuCardCount > 0)
							{
								bUpdate = CT_TRUE;
								itSecond->second.cbHuCard_DisReconect[i].cbCardCount -= cbCanHuCardCount;
							}
						}

						if(bUpdate)
						{
							CMD_S_HaveHuCard_Ting data;
							data.cbBei = std::pow(2, itSecond->second.wJiaBei);
							data.wCurrentUser = itSecond->first;
							for(int i = 0; i < MAX_HU_CARD_COUNT; i++)
							{
								if(itSecond->second.cbHuCard_DisReconect[i].cbCardId != INVALID_CARD_DATA)
								{
									data.cbHuCard[i].cbCardId = itSecond->second.cbHuCard_DisReconect[i].cbCardId;
									data.cbHuCard[i].cbCardCount = itSecond->second.cbHuCard_DisReconect[i].cbCardCount;
									data.cbHuCard[i].cbFanCount = itSecond->second.cbHuCard_DisReconect[i].cbFanCount;
								}
								if(data.cbHuCard[i].cbCardCount > 4)
								{
                                    data.cbHuCard[i].cbCardCount = 0;
								}
							}

							for(CT_DWORD p= 0; p < GAME_PLAYER_MAX; p++)
                            {
							    if(p == m_dwBanker)
                                {
							        if(bSendTipData)
                                    {
                                        SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                                    }
                                }
                                else
                                {
                                    if(bSendTipData)
                                    {
                                        SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                                    }
                                }
                            }
						}
                    }
                }

				return;
			}
		}

		CMD_S_HaveHuCard_Ting data;
		data.cbBei = std::pow(2, it->second.wJiaBei);
		data.wCurrentUser = dwChairID;
		memset(data.cbHuCard, INVALID_CARD_DATA, sizeof(data.cbHuCard));
		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == dwChairID)
			{
                SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
			}
			else
			{
			    if(bSendTipData)
                {
                    SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                }
			}
		}
	}
	else
	{
		CMD_S_HaveHuCard_Ting data;
		data.cbBei = std::pow(2,it->second.wJiaBei);
		data.wCurrentUser = dwChairID;
		memset(data.cbHuCard, INVALID_CARD_DATA, sizeof(data.cbHuCard));
		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == dwChairID)
			{
                SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
			}
			else
			{
			    if(bSendTipData)
                {
                    SendTableData(p, SUB_S_MABY_HU_CARD_TING, &data, sizeof(data), false);
                }
			}
		}
	}
}

CT_VOID CGameProcess::TipPlayerMax(CT_DWORD dwChair)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChair);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "通过椅子ID ：" << dwChair << " 找不到玩家";
		return;
	}

	if(!(it->second.cbPlayerAction & WIK_TING_MY) && (!it->second.bIsTing))
	{
		LOG(ERROR) << "玩家 ：" << it->second.dwUserID << "当前不能听牌，只有在玩家能听牌的时候才能调用TipPlayerMax";
		return;
	}

	if( 0 == it->second.cbListenCardCount)
	{
		LOG(ERROR) << "玩家: "<<it->second.dwUserID << "能听的张数为0，不能调用TipPlayerMax";
		return;
	}

	//最大剩余张数 和 最大的番数、
	CT_WORD wMaxShengYuZhangCnt = 0, wMaxHuFan = 0;
	//玩家的最大和最多信息
	std::vector<tagPlayerMaxInfo> vecPlayerMaxInfo;
	vecPlayerMaxInfo.clear();

	CT_WORD wLoop = it->second.cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : it->second.cbListenCardCount;
	tagListenCardData *pListenData = NULL;
	//这个for循环得出玩家打出一张能胡的牌时能胡的牌剩余张数和最大番数
	for(CT_WORD i = 0 ; i < wLoop ;i++)
	{
		pListenData = (tagListenCardData *)it->second.cbListenCardData[i];
		if(pListenData)
		{
            tagPlayerMaxInfo maxInfo;
            maxInfo.maxInfo = Max_Normal;
            maxInfo.cbCardData = pListenData->cbOutCard;

			CT_WORD wLoopHuCard = pListenData->cbHuCardCount > MAX_INDEX ? MAX_INDEX : pListenData->cbHuCardCount;
			//打这张牌后胡的剩余牌的总数
			CT_WORD wTempRemainTotalCount = 0;
			//打这张牌后胡的最大番数
			CT_WORD wTempHuMaxFan = 0;
			for(CT_WORD j = 0; j < wLoopHuCard; j++)
			{
				tagHuCardData *pHuCardData = (tagHuCardData *)(pListenData->cbHuCardData + j * sizeof(tagHuCardData));
				if(pHuCardData)
				{
				    //修正pHuCardData->cbRemainCount
				    pHuCardData->cbRemainCount = GetShengYuCountAfterTing(dwChair, pHuCardData->cbListenCard);

					wTempRemainTotalCount += pHuCardData->cbRemainCount;
					if(pHuCardData->wMultipleCount > wMaxHuFan && pHuCardData->cbRemainCount > 0)
					{
						wMaxHuFan = pHuCardData->wMultipleCount;
					}
					if(pHuCardData->wMultipleCount > wTempHuMaxFan && pHuCardData->cbRemainCount > 0)
					{
						wTempHuMaxFan = pHuCardData->wMultipleCount;
						maxInfo.wShengYuCount_wHuMaxFan = pHuCardData->cbRemainCount;
					}
				}
			}
            if(wTempRemainTotalCount > wMaxShengYuZhangCnt)
            {
                wMaxShengYuZhangCnt = wTempRemainTotalCount;
            }

            maxInfo.wTotalShengYuCount = wTempRemainTotalCount;
            maxInfo.wHuMaxFan = wTempHuMaxFan;
            vecPlayerMaxInfo.push_back(maxInfo);
		}
	}

	if(vecPlayerMaxInfo.size() == 1)
	{
		CMD_S_OutType_Tip tip;
		tip.bOutCardData[0] = vecPlayerMaxInfo[0].cbCardData;
		tip.bCanHuType[0] = STATE_OUT_NORMAL;

		SendTableData(dwChair, SUB_S_OUT_TYPE_TIP, &tip, sizeof(tip), true);
	}
	else if(vecPlayerMaxInfo.size() > 1)
	{
	    //解决最大和最多是同一张的情况:优先设置为最大
        {
            for(size_t i = 0; i < vecPlayerMaxInfo.size(); i++)
            {
                //如果出一张牌，胡的番数既是最大  能胡的剩余张数又是最多，就只设置最大
                if(vecPlayerMaxInfo[i].wHuMaxFan == wMaxHuFan && wMaxHuFan > 0)
                {
                    vecPlayerMaxInfo[i].maxInfo = Max_Fan;
                }
                else if(vecPlayerMaxInfo[i].wTotalShengYuCount == wMaxShengYuZhangCnt && wMaxShengYuZhangCnt > 0)
                {
                    vecPlayerMaxInfo[i].maxInfo = Max_HuCardCount;
                }
                else
                {
                    vecPlayerMaxInfo[i].maxInfo = Max_Normal;
                }
            }
        }

		//解决能胡最大番数有两张以及以上的情况:最大番数相同就以剩余张数最多为大。
        {
            CT_BYTE maxtotalshengyu = 0;
            CT_BYTE maxshengyu = 0;
            for(size_t i = 0; i < vecPlayerMaxInfo.size(); i++)
            {
                if(vecPlayerMaxInfo[i].maxInfo == Max_Fan && vecPlayerMaxInfo[i].wTotalShengYuCount > maxtotalshengyu)
                {
                    //胡最大番数时，能胡的剩余总张数
                    maxtotalshengyu = vecPlayerMaxInfo[i].wTotalShengYuCount;
                }
                if(vecPlayerMaxInfo[i].maxInfo == Max_Fan && vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan > maxshengyu)
                {
                    //胡最大番数时，胡最大番的那张牌剩余张数
                    maxshengyu = vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan;
                }
            }

            for(size_t i = 0; i < vecPlayerMaxInfo.size(); i++)
            {
                if(vecPlayerMaxInfo[i].maxInfo == Max_Fan && vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan != maxshengyu)
                {
                    //最大番有多张时，能胡最大番的牌剩余张数最多的即是最大，其他能胡最大番数的不是最大
                    if(vecPlayerMaxInfo[i].wTotalShengYuCount == wMaxShengYuZhangCnt)
                    {
                        //如果胡的番数最大但是剩余牌少，但是能胡的总剩余牌数等于最大总剩余张数，那么就从最大降级为最多
                        vecPlayerMaxInfo[i].maxInfo = Max_HuCardCount;
                    }
                    else
                    {
                        vecPlayerMaxInfo[i].maxInfo = Max_Normal;
                    }
                }

                if(vecPlayerMaxInfo[i].maxInfo == Max_Fan && vecPlayerMaxInfo[i].wTotalShengYuCount != maxtotalshengyu)
                {
                    //能到这里说明能胡最大番数的牌有多张，并且它们的剩余张数也相同，那么就以总剩余张数为标准，总剩余张数最多即为最大
                    if(vecPlayerMaxInfo[i].wTotalShengYuCount == wMaxShengYuZhangCnt)
                    {
                        vecPlayerMaxInfo[i].maxInfo = Max_HuCardCount;
                    }
                    else
                    {
                        vecPlayerMaxInfo[i].maxInfo = Max_Normal;
                    }
                }
            }
        }

        {
            //解决能胡剩余张数最多有两张以及以上的情况:剩余张数最多相同就以能胡最大番数为多
            CT_BYTE maxfan = 0;
            CT_BYTE maxshengyu = 0;
            for(size_t i = 0; i < vecPlayerMaxInfo.size(); i++)
            {
                if(vecPlayerMaxInfo[i].maxInfo == Max_HuCardCount && vecPlayerMaxInfo[i].wHuMaxFan > maxfan)
                {
                    maxfan = vecPlayerMaxInfo[i].wHuMaxFan;
                }
                if(vecPlayerMaxInfo[i].maxInfo == Max_HuCardCount && vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan > maxshengyu)
                {
                    maxshengyu = vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan;
                }
            }
            for(size_t i = 0; i < vecPlayerMaxInfo.size(); i++)
            {
                if(vecPlayerMaxInfo[i].maxInfo == Max_HuCardCount && vecPlayerMaxInfo[i].wHuMaxFan != maxfan)
                {
                    //虽然能胡的剩余张数最多，但是胡的最大番数没有另外一张（能胡的剩余张数也是最多）大，所以剥夺其最多剩余张数
                    vecPlayerMaxInfo[i].maxInfo = Max_Normal;
                }
                if(vecPlayerMaxInfo[i].maxInfo == Max_HuCardCount && vecPlayerMaxInfo[i].wShengYuCount_wHuMaxFan != maxshengyu)
                {
                    vecPlayerMaxInfo[i].maxInfo = Max_Normal;
                }
            }

        }

		size_t loop = vecPlayerMaxInfo.size() > MAX_CARD_HAND ? MAX_CARD_HAND : vecPlayerMaxInfo.size();
		CMD_S_OutType_Tip tip;
		for(size_t i = 0; i < loop; i++)
		{
			tip.bOutCardData[i] = vecPlayerMaxInfo[i].cbCardData;

			if(vecPlayerMaxInfo[i].maxInfo == Max_Normal)
			{
				tip.bCanHuType[i] = STATE_OUT_NORMAL;
			}
			else if(vecPlayerMaxInfo[i].maxInfo == Max_HuCardCount)
			{
				tip.bCanHuType[i] = STATE_OUT_NUN_MAX;
			}
			else if(vecPlayerMaxInfo[i].maxInfo == Max_Fan)
			{
				tip.bCanHuType[i] = STATE_OUT_FAN_MAX;
			}
			else
			{
				LOG(ERROR) << "提示玩家：" << m_pGameDeskPtr->GetUserID(dwChair) << " 最大的时候, maxinfo未知:"<< vecPlayerMaxInfo[i].maxInfo;
			}
		}

		SendTableData(dwChair, SUB_S_OUT_TYPE_TIP, &tip, sizeof(tip), true);
	}
}

CT_BOOL CGameProcess::OnMsg_PlayerOperate_ByZhuaCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize)
{
	if(m_cbCurState != STATE_WAIT_SEND_OPERATE && m_cbCurState != STATE_WAIT_OUT)
	{
		LOG(ERROR) << "玩家: "<< m_pGameDeskPtr->GetUserID(wChairID) << " 执行抓牌后操作，但是牌局状态是: m_cbCurState:"<<(int)m_cbCurState;
		return CT_TRUE;
	}

	if(dwDataSize != sizeof(CMD_C_OperateSend))
	{
		LOG(ERROR) << "dwDataSize:" <<dwDataSize << " is not equal  to:" << sizeof(CMD_C_OperateSend);
		return CT_TRUE;
	}

	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "OnMsg_PlayerOperate_ByZhuaCard 玩家操作，通过椅子ID: "<< wChairID << " 找不到玩家";
		return CT_TRUE;
	}

    CMD_C_OperateSend *pData = (CMD_C_OperateSend *)pDataBuffer;

	if(m_dwCurOperatePlayer != wChairID)
	{
		LOG(ERROR) << "玩家: "<< it->second.dwUserID << " 执行抓牌后引发的操作,但是目前应该是玩家: "<< m_pGameDeskPtr->GetUserID(m_dwCurOperatePlayer) << "执行这项操作";
		return CT_TRUE;
	}

	if(m_bCanTianTingBanker && wChairID == m_dwBanker && !(pData->wOperateCode & WIK_TING_MY))
	{
		//庄家能天听的情况下却选择的不听
		m_bCanTianTingBanker = CT_FALSE;
	}

	if(m_bCanTianTingXian && wChairID != m_dwBanker && !(pData->wOperateCode & WIK_TING_MY))
	{
		m_bCanTianTingXian = CT_FALSE;
	}

	//抓牌后玩家只能执行的操作: 暗杠 巴杠 自摸胡 过牌 服务器超时过牌，其他操作都是非法的。
	if( !(pData->wOperateCode & WIK_GANG_AN_MY) && !(pData->wOperateCode & WIK_GANG_MING_PENG_MY) && !(pData->wOperateCode & WIK_HU_MY) &&
            (pData->wOperateCode != WIK_NULL_MY) && (pData->wOperateCode != WIK_NULL_MY_TIMEOUT) && !(pData->wOperateCode & WIK_TING_MY))
    {
	    LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << "在抓牌后执行了非法的操作: "<< pData->wOperateCode;
        return  CT_TRUE;
    }

    //玩家是否能进行操作
    if(!(pData->wOperateCode & it->second.cbPlayerAction) && (pData->wOperateCode != WIK_NULL_MY) && (pData->wOperateCode != WIK_NULL_MY_TIMEOUT))
    {
        LOG(ERROR) << "玩家ID:" << it->second.dwUserID << " 能够进行的操作:" << it->second.cbPlayerAction << " 但是玩家却进行了非法操作: "<< pData->wOperateCode;
        return CT_TRUE;
    }

	//是否能暗杠
    if(pData->wOperateCode & WIK_GANG_AN_MY)
    {
        CT_BYTE cbHandCardIndex = m_GameLogic.SwitchToCardIndex(pData->cbCardId);
        CT_BYTE cbHandCardCnt = m_GameLogic.GetCardCount(it->second.cbHandCardIndex,cbHandCardIndex);
        if(cbHandCardCnt != 4)
        {
            LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << " 想暗杠牌："<< (int)pData->cbCardId << " 但是玩家手中只有：" <<(int)cbHandCardCnt << " 张";
            return CT_TRUE;
        }
    }

    //是否能巴杠
    if(pData->wOperateCode & WIK_GANG_MING_PENG_MY)
    {
        CT_BYTE cbHandCardIndex = m_GameLogic.SwitchToCardIndex(pData->cbCardId);
        CT_BYTE cbHandCardCnt = m_GameLogic.GetCardCount(it->second.cbHandCardIndex,cbHandCardIndex);
        if(cbHandCardCnt != 1)
        {
            LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << " 想巴杠牌："<< (int)pData->cbCardId << " 但是玩家手中只有：" <<(int)cbHandCardCnt << " 张";
            return CT_TRUE;
        }

        CT_BOOL bFindBaGang = CT_FALSE;
        for(int i = 0; i < MAX_WEAVE ;i++)
        {
            if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY && it->second.weaveItem[i].cbCenterCard == pData->cbCardId)
            {
                bFindBaGang = CT_TRUE;
                break;
            }
        }
        if(bFindBaGang == CT_FALSE)
        {
            LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << " 想巴杠牌："<< (int)pData->cbCardId << " 但是玩家手中没有碰牌";
            return CT_TRUE;
        }
    }

    //是否能胡
    if(pData->wOperateCode & WIK_HU_MY)
    {
        if(!(pData->wOperateCode & it->second.cbPlayerAction))
        {
            LOG(ERROR)  << "玩家ID: "<< it->second.dwUserID << " 不能胡，但是客户端却发送了胡牌命令";
            return CT_TRUE;
        }
    }

    //是否能听
    if(pData->wOperateCode & WIK_TING_MY)
	{
    	if(!(pData->wOperateCode & it->second.cbPlayerAction))
		{
    		LOG(ERROR) << "玩家ID: " << it->second.dwUserID << " 不能听，但是客户端发送了听牌命令";
    		return CT_TRUE;
		}
		if(it->second.bIsTing)
		{
			LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << " 已经听牌,但是客户端发送了听牌消息.";
			return CT_TRUE;
		}
	}

    //实际的碰胡操作
	if(it->second.cbPlayerAction != WIK_NULL_MY)
	{
		CT_BOOL bOperateSucc = CT_FALSE;

		if(pData->wOperateCode == WIK_NULL_MY)
		{
			bOperateSucc = CT_TRUE;
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 点击过牌11";
		}
		else if(pData->wOperateCode == WIK_NULL_MY_TIMEOUT)
		{
			bOperateSucc = CT_TRUE;
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 超时过牌11";
		}
		else if(pData->wOperateCode & it->second.cbPlayerAction)
		{
			if(pData->wOperateCode & WIK_GANG_MY)
			{
				m_bGangStatus = CT_TRUE;
                bOperateSucc = PlayerGangCard(wChairID, pData->cbCardId, pData->wOperateCode);
				if(bOperateSucc)
				{
					for(MapChairInfo::iterator itTipS = m_mapChairInfo.begin(); itTipS != m_mapChairInfo.end(); itTipS++)
					{
						if(itTipS->first == wChairID)
						{
							continue;
						}
						if(itTipS->second.bIsTing == CT_FALSE)
						{
							break;
						}

						for(int k = 0; k < MAX_HU_CARD_COUNT; k++)
						{
							if(itTipS->second.cbCanHuCard_AfterTing[k] == INVALID_CARD_DATA)
							{
								continue;
							}

							if(itTipS->second.cbCanHuCard_AfterTing[k] == pData->cbCardId)
							{
								if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, pData->cbCardId, CT_FALSE, CT_FALSE);
								}
								else
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, pData->cbCardId, CT_FALSE, CT_FALSE, CT_FALSE);
								}
							}
						}
					}

					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 杠牌1:" << (int)pData->cbCardId;
				}
			}
			else if(pData->wOperateCode & WIK_HU_MY)
			{
				bOperateSucc = PlayerChiHuCard(wChairID, it->second.cbPlayerActionCardData, CT_TRUE);
				if(bOperateSucc)
					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 胡牌:" << (int)it->second.cbPlayerActionCardData;
			}
			else if(pData->wOperateCode & WIK_TING_MY)
			{
				bOperateSucc = PlayerTingCard(wChairID);
				if(bOperateSucc)
					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 听牌成功";
			}
			else
			{
				bOperateSucc = CT_FALSE;
				LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 执行抓牌后的行为: "<< pData->wOperateCode << "错误";
			}
		}
		else
		{
			bOperateSucc = CT_FALSE;
			LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 执行抓牌后的行为11: "<< pData->wOperateCode << "错误";
		}

		if(bOperateSucc)
		{
			m_cbCurState = STATE_DEFAUT;

            //服务器超时过，给客户端发送取消提示消息
            CMD_S_OperateTip_Cancel cancel;
            cancel.wOperateCode = WIK_NULL_MY;
            SendTableData(wChairID, SUB_S_OPERATE_TIP_CANCEL, &cancel, sizeof(cancel), true);

			if (pData->wOperateCode != WIK_NULL_MY && pData->wOperateCode != WIK_NULL_MY_TIMEOUT && !(pData->wOperateCode & WIK_TING_MY))
			{
				//点听按钮不会广播听
				CMD_S_OperateSend tip;
				tip.wOperateCode = pData->wOperateCode;
				tip.wOperateUser = wChairID;
				tip.cbCardId = pData->cbCardId;

				for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
				{
					if(p == m_dwBanker)
					{
						SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), true);
					}
					else
					{
						SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), false);
					}
				}
			}

			//如果是听牌,必须放在107后面
			if(pData->wOperateCode == WIK_TING_MY)
			{
			    memset(it->second.cbMustOutCard_AfterClickTing, INVALID_CARD_DATA, MAX_CARD_HAND*sizeof(CT_BYTE));
				//提示出哪些牌可以听牌
				CMD_S_OutTip_Ting ting;
				CT_WORD  wLoop = it->second.cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : it->second.cbListenCardCount;
				for(int i = 0; i < wLoop; i++)
				{
					tagListenCardData *pListen = (tagListenCardData *)it->second.cbListenCardData[i];
					if(pListen && pListen->cbOutCard != INVALID_CARD_DATA)
					{
						ting.bOutCardData[i] = pListen->cbOutCard;
						it->second.cbMustOutCard_AfterClickTing[i] = pListen->cbOutCard;
					}
				}
				SendTableData(wChairID, SUB_S_OUT_TIP_TING, &ting, sizeof(ting), true);
				LOG_IF(WARNING, PRINT_LOG_INFO) << "1给玩家: " << m_pGameDeskPtr->GetUserID(wChairID) << " 发送114";

				//取消听牌触发了听牌就不要再提示出牌了否则会刷新定时器
				if(m_eTingReason != TingTrigger::CancelTing)
				{
					//提示出牌,用于听后玩家点击出牌，或者听后玩家不点击出牌超时出牌，如果是超时出牌，INVALID_CARD_DATA传这个值是可以的.
					//点击听牌按钮后，不点击取消按钮，也不出牌，那么这个函数就会超时出牌，而且出的那张牌可以让玩家胡牌。
					TipPlayerOutCard(wChairID, INVALID_CARD_DATA, CT_TRUE);
				}
				else
                {
				    //取消听牌触发了听牌，玩家点了听牌，要允许玩家出牌
                    m_cbCurState = STATE_WAIT_OUT;
                }

				//点击听牌后再发一次 115
				TipPlayerMax(wChairID);
			}

			SetMaxActionData(wChairID, pData->wOperateCode);

			//玩家操作完成,清除玩家的行为数据
			it->second.clearPlayerAction();

			//所有操作完成进行下一步
/*			CT_BOOL bIsHave = CT_FALSE;
			for (MapChairInfo::iterator itAll = m_mapChairInfo.begin(); itAll != m_mapChairInfo.end(); itAll++)
			{
				if (itAll->second.cbPlayerAction != WIK_NULL_MY)
				{
					bIsHave = CT_TRUE;
					break;
				}
			}*/

			//抓牌后引发的行为,所有玩家都响应了
			m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD);

			CT_BOOL bQiangGangHu = CT_FALSE;
			if(pData->wOperateCode & WIK_GANG_MY && pData->wOperateCode & WIK_GANG_MING_PENG_MY)
			{
				//如果巴杠操作成功，就判断其他玩家是否能抢杠胡
				bQiangGangHu = IsHaveQiangGangHu_AfterBaGang(wChairID, pData->cbCardId);
			}

			if(bQiangGangHu)
			{
				m_cbCurState = STATE_WAIT_OUT_OPERATE;
				m_startOperateTime = Utility::GetTick();
				//提示抢杠胡
				TipPlayerAction(wChairID, pData->cbCardId, Trigger_OutCard);
			}
			else
			{
				//不能抢杠胡就走以前的流程
				if (m_dwMaxAction == WIK_NULL_MY || m_dwMaxAction == WIK_NULL_MY_TIMEOUT)
				{
					if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing == CT_FALSE)
					{
						//如果听牌了就自动打牌
						m_dwAutoOutCardPlayer = wChairID;
						m_dWAutoOutCardData = m_cbZhuaCardData;
						m_dwCurOperatePlayer = wChairID;
						m_cbCurState = STATE_WAIT_OUT;
						m_pGameDeskPtr->SetGameTimer(IDI_AUTO_OUT_CARD_AFTER_TING, TIMER_AUTO_OUT_CARD_AFTER_TING);
					}
					else
					{
					    if(m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, m_cbZhuaCardData) > 0)
                        {
                            //抓牌行为引发了抓牌者的操作，但是他点了过,那就通知他出牌
                            TipPlayerOutCard(wChairID, m_cbZhuaCardData, CT_TRUE);
                        }
                        else
                        {
                            for (int k = MAX_INDEX - 1; k >= 0; k--)
                            {
                                if (it->second.cbHandCardIndex[k] > 0)
                                {
                                    CT_BYTE cbTempCardData = m_GameLogic.SwitchToCardData(k);
                                    TipPlayerOutCard(wChairID, cbTempCardData, CT_TRUE);
                                    break;
                                }
                            }
                        }
					}
				}
				else
				{
					//当前二人麻将，所以作出行为的玩家就是最大行为玩家
					PlayerFinishAction(m_dwMaxActionPlayer, m_dwMaxAction);
				}
			}
		}
/*		else
		{
			LOG(ERROR)<<"玩家ID:" << m_pGameDeskPtr->GetUserID(wChairID) << " 当前执行的操作：" << (int)pData->wOperateCode << " 能执行的操作:" << (int)it->second.cbPlayerAction;
		}*/
	}
	else
	{
		LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(wChairID) << " 能执行的行为: "<<(int)it->second.cbPlayerAction << " 玩家的执行牌数据: "<< (int)it->second.cbPlayerActionCardData;
	}

	return CT_TRUE;
}

CT_BOOL CGameProcess::PlayerJiaBei(CT_DWORD dwPlayeChair)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwPlayeChair);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "PlayerJiaBei 通过椅子ID:"<<dwPlayeChair<<" 找不到玩家了";
		return CT_FALSE;
	}

	it->second.wJiaBei++;

    CMD_S_Double_Ting jiabei;
    jiabei.wOperateCode = WIK_JIABEI;
    jiabei.wOperateUser = dwPlayeChair;
    jiabei.cbBei = std::pow(2,it->second.wJiaBei);

    for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
	{
    	if(p == m_dwBanker)
		{
			SendTableData(p, SUB_S_DOUBLUE_TING, &jiabei, sizeof(jiabei), true);
		}
		else
		{
			SendTableData(p, SUB_S_DOUBLUE_TING, &jiabei, sizeof(jiabei), false);
		}
	}

    return CT_TRUE;
}

CT_BOOL CGameProcess::OnMsg_PlayerOperate_ByOutCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize)
{
	if(m_cbCurState != STATE_WAIT_OUT_OPERATE)
	{
		LOG(ERROR) << "玩家: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 执行出牌后操作,但是目前牌局状态 m_cbCurState:"<<(int)m_cbCurState;
		return CT_TRUE;
	}

	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家操作时通过椅子ID："  << wChairID << "找不到玩家了";
		return CT_TRUE;
	}


	if(dwDataSize != sizeof(CMD_C_OperateOut))
	{
		LOG(ERROR) << "dwDataSize:" << dwDataSize << " is not equal to:" << sizeof(CMD_C_OperateOut);
		return CT_TRUE;
	}

	CMD_C_OperateOut *pData = (CMD_C_OperateOut *)pDataBuffer;

	if(m_dwCurOperatePlayer != wChairID)
	{
		LOG(ERROR) << "玩家:"<<it->second.dwUserID << " 执行出牌后引发的操作时,应该执行这项操作的玩家应该是: "<<m_pGameDeskPtr->GetUserID(m_dwCurOperatePlayer);
		return CT_TRUE;
	}

	//玩家出牌后的操作只能有: 过 服务器超时过 碰 明杠 吃 胡， 其他操作都是非法
	if( (pData->wOperateCode != WIK_NULL_MY) && (pData->wOperateCode != WIK_NULL_MY_TIMEOUT) && !(pData->wOperateCode & WIK_PENG_MY) &&
            !(pData->wOperateCode & WIK_GANG_MING_MY) && !(pData->wOperateCode & WIK_CHI_MY) && !(pData->wOperateCode & WIK_HU_MY))
    {
	    LOG(ERROR) << "玩家ID:"<<it->second.dwUserID << "在抓牌后执行了非法操作:" << pData->wOperateCode;
	    return CT_TRUE;
    }

    //玩家是否能进行操作
    if(!(pData->wOperateCode & it->second.cbPlayerAction) && (pData->wOperateCode != WIK_NULL_MY) && (pData->wOperateCode != WIK_NULL_MY_TIMEOUT))
    {
        LOG(ERROR) << "玩家ID:" << it->second.dwUserID << " 能够进行的操作:" << it->second.cbPlayerAction << " 但是玩家却进行了非法操作: "<< pData->wOperateCode;
        return CT_TRUE;
    }

    //是否能碰
    if(pData->wOperateCode & WIK_PENG_MY)
    {
        CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(it->second.cbPlayerActionCardData);
        CT_BYTE cbCardCount = m_GameLogic.GetCardCount(it->second.cbHandCardIndex, cbCardIndex);
        if(cbCardCount < 2)
        {
            LOG(ERROR) << "玩家ID: "<<it->second.dwUserID << " 碰的牌是: "<<(int)it->second.cbPlayerActionCardData << " 但是手中只有:"<< (int)cbCardCount<<" 张";
            return CT_FALSE;
        }
    }
    //是否能明杠
    if(pData->wOperateCode & WIK_GANG_MING_MY)
    {
        CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(it->second.cbPlayerActionCardData);
        CT_BYTE cbCardCount = m_GameLogic.GetCardCount(it->second.cbHandCardIndex, cbCardIndex);
        if(cbCardCount != 3)
        {
            LOG(ERROR) << "玩家ID: "<<it->second.dwUserID << " 明杠的牌是: "<<(int)it->second.cbPlayerActionCardData << " 但是手中只有:"<< (int)cbCardCount<<" 张";
            return CT_FALSE;
        }
    }
    //是否能吃
    if(pData->wOperateCode & WIK_LEFT_MY)
    {
        CT_BYTE cbCenterCard = it->second.cbPlayerActionCardData;
        CT_BYTE cbCard1 = cbCenterCard + 1;
        CT_BYTE cbCard2 = cbCenterCard + 2;
        CT_BYTE cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard1);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "左吃的时候，没有牌: "<< (int)cbCard1;
			return CT_TRUE;
		}
		cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard2);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "左吃的时候，没有牌: "<< (int)cbCard2;
			return CT_TRUE;
		}
    }
    if(pData->wOperateCode & WIK_CENTER_MY)
	{
		CT_BYTE cbCenterCard = it->second.cbPlayerActionCardData;
		CT_BYTE cbCard1 = cbCenterCard - 1;
		CT_BYTE cbCard2 = cbCenterCard + 1;
		CT_BYTE cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard1);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "中吃的时候，没有牌: "<< (int)cbCard1;
			return CT_TRUE;
		}
		cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard2);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "中吃的时候，没有牌: "<< (int)cbCard2;
			return CT_TRUE;
		}
	}
	if(pData->wOperateCode & WIK_RIGHT_MY)
	{
		CT_BYTE cbCenterCard = it->second.cbPlayerActionCardData;
		CT_BYTE cbCard1 = cbCenterCard - 1;
		CT_BYTE cbCard2 = cbCenterCard - 2;
		CT_BYTE cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard1);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "右吃的时候，没有牌: "<< (int)cbCard1;
			return CT_TRUE;
		}
		cbCnt = m_GameLogic.GetCardCountByCardData(it->second.cbHandCardIndex, cbCard2);
		if(cbCnt == 0)
		{
			LOG(ERROR) << "玩家ID: " << it->second.dwUserID << "右吃的时候，没有牌: "<< (int)cbCard2;
			return CT_TRUE;
		}
	}

	//是否能胡
	if(pData->wOperateCode & WIK_HU_MY)
	{
		if(!(pData->wOperateCode & it->second.cbPlayerAction))
		{
			LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << " 在出牌后发送胡牌命令，但是玩家不能胡，玩家能进行的操作:" << it->second.cbPlayerAction;
			return CT_TRUE;
		}
	}

	//实际的吃碰杠胡操作
	if(it->second.cbPlayerAction != WIK_NULL_MY)
	{
		CT_BOOL bOperateSucc = CT_FALSE;

		if(pData->wOperateCode == WIK_NULL_MY)
		{
			bOperateSucc = CT_TRUE;
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 点击过牌:" << (int)it->second.cbPlayerActionCardData;
		}
		else if(pData->wOperateCode == WIK_NULL_MY_TIMEOUT)
		{
			bOperateSucc = CT_TRUE;
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 超时过牌:" << (int)it->second.cbPlayerActionCardData;
		}
		else if(pData->wOperateCode & it->second.cbPlayerAction)
		{
			//碰
			if(pData->wOperateCode & WIK_PENG_MY)
			{
				bOperateSucc = PlayerPengCard(wChairID,it->second.cbPlayerActionCardData);
				if(bOperateSucc)
				{
					for(MapChairInfo::iterator itTipS = m_mapChairInfo.begin(); itTipS != m_mapChairInfo.end(); itTipS++)
					{
						if(itTipS->first == wChairID)
						{
							continue;
						}
						if(itTipS->second.bIsTing == CT_FALSE)
						{
							break;
						}

						for(int k = 0; k < MAX_HU_CARD_COUNT; k++)
						{
							if(itTipS->second.cbCanHuCard_AfterTing[k] == INVALID_CARD_DATA)
							{
								continue;
							}

							if(itTipS->second.cbCanHuCard_AfterTing[k] == it->second.cbPlayerActionCardData)
							{
								if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, it->second.cbPlayerActionCardData, CT_FALSE, CT_FALSE);
								}
								else
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, it->second.cbPlayerActionCardData, CT_FALSE, CT_FALSE, CT_FALSE);
								}
							}
						}
					}
					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 碰牌:" << (int)it->second.cbPlayerActionCardData;
				}
			}
			//杠
			else if(pData->wOperateCode & WIK_GANG_MY)
			{
				m_bGangStatus = CT_TRUE;
				bOperateSucc = PlayerGangCard(wChairID, it->second.cbPlayerActionCardData, pData->wOperateCode);
				if(bOperateSucc)
				{
					for(MapChairInfo::iterator itTipS = m_mapChairInfo.begin(); itTipS != m_mapChairInfo.end(); itTipS++)
					{
						if(itTipS->first == wChairID)
						{
							continue;
						}
						if(itTipS->second.bIsTing == CT_FALSE)
						{
							break;
						}

						for(int k = 0; k < MAX_HU_CARD_COUNT; k++)
						{
							if(itTipS->second.cbCanHuCard_AfterTing[k] == INVALID_CARD_DATA)
							{
								continue;
							}

							if(itTipS->second.cbCanHuCard_AfterTing[k] == it->second.cbPlayerActionCardData)
							{
								if(m_dwTingCardPlayerCnt >= GAME_PLAYER) {
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, it->second.cbPlayerActionCardData,
																   CT_FALSE, CT_FALSE);
								}
								else
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, it->second.cbPlayerActionCardData, CT_FALSE, CT_FALSE, CT_FALSE);
								}
							}
						}
					}

					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 杠牌2:" << (int)it->second.cbPlayerActionCardData;
				}
			}
			//吃
			else if(pData->wOperateCode & WIK_CHI_MY)
			{
				bOperateSucc = PlayerChiCard(wChairID, it->second.cbPlayerActionCardData, pData->wOperateCode);
				if(bOperateSucc)
				{
					for(MapChairInfo::iterator itTipS = m_mapChairInfo.begin(); itTipS != m_mapChairInfo.end(); itTipS++)
					{
						if(itTipS->first == wChairID)
						{
							continue;
						}
						if(itTipS->second.bIsTing == CT_FALSE)
						{
							break;
						}

						CT_BYTE cbActionCard = it->second.cbPlayerActionCardData;
						CT_BYTE cbCard1 = INVALID_CARD_DATA, cbCard2 = INVALID_CARD_DATA, cbCard3 = INVALID_CARD_DATA;
						if(pData->wOperateCode & WIK_LEFT_MY)
						{
							cbCard1 = cbActionCard;
							cbCard2 = cbCard1 + 1;
							cbCard3 = cbCard1 + 2;
						}
						else if(pData->wOperateCode & WIK_CENTER_MY)
						{
							cbCard1 = cbActionCard - 1;
							cbCard2 = cbActionCard;
							cbCard3 = cbActionCard + 1;
						}
						else if(pData->wOperateCode & WIK_RIGHT_MY)
						{
							cbCard1 = cbActionCard - 2;
							cbCard2 = cbActionCard - 1;
							cbCard3 = cbActionCard;
						}

						for(int k = 0; k < MAX_HU_CARD_COUNT; k++)
						{
							if(itTipS->second.cbCanHuCard_AfterTing[k] == INVALID_CARD_DATA)
							{
								continue;
							}

							if(itTipS->second.cbCanHuCard_AfterTing[k] == cbCard1 ||
							itTipS->second.cbCanHuCard_AfterTing[k] == cbCard2 ||
							itTipS->second.cbCanHuCard_AfterTing[k] == cbCard3)
							{
								if(m_dwTingCardPlayerCnt >= GAME_PLAYER) {
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, itTipS->second.cbCanHuCard_AfterTing[k], CT_FALSE, CT_FALSE);
								}
								else
								{
									TipPlayerMayBeHuCard_AfterTing(itTipS->first, itTipS->second.cbCanHuCard_AfterTing[k], CT_FALSE, CT_FALSE, CT_FALSE);
								}
							}
						}
					}
					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 吃牌:" << (int) it->second.cbPlayerActionCardData;
				}
			}
			//胡
			else if(pData->wOperateCode & WIK_HU_MY)
			{
				bOperateSucc = PlayerChiHuCard(wChairID, it->second.cbPlayerActionCardData,CT_FALSE);
				if(bOperateSucc)
					LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:" << it->second.dwUserID << " 胡牌:" << (int)it->second.cbPlayerActionCardData;
			}
			else
			{
				bOperateSucc = CT_FALSE;
				LOG(ERROR) << "玩家ID:" << m_pGameDeskPtr->GetUserID(wChairID)<<" 未处理的操作类型:" << pData->wOperateCode;
			}
		}
		else
		{
			bOperateSucc = CT_FALSE;
			LOG(ERROR) << "玩家ID:" << m_pGameDeskPtr->GetUserID(wChairID) << " 当前能作出的行为:"<<(int)it->second.cbPlayerAction
			<< " 行为描述:" << CanActionStr(it->second.cbPlayerAction).c_str() << " 不包括行为:"<< pData->wOperateCode << " 行为描述："<< CanActionStr(pData->wOperateCode).c_str();
		}

		if(bOperateSucc)
		{
			m_cbCurState = STATE_DEFAUT;
			//在清除玩家是否能抢杠胡之前获取
			CT_BOOL bCanQiangGangHu = it->second.bCanQiangGangHu;

			//if(pData->wOperateCode == WIK_NULL_MY_TIMEOUT)
			//{
				//服务器超时过，给客户端发送取消提示消息
				CMD_S_OperateTip_Cancel cancel;
				cancel.wOperateCode = WIK_NULL_MY;
				SendTableData(wChairID, SUB_S_OPERATE_TIP_CANCEL, &cancel, sizeof(cancel), true);
			//}

			if(pData->wOperateCode != WIK_NULL_MY && pData->wOperateCode != WIK_NULL_MY_TIMEOUT)
			{
				//如果不是过就广播
				CMD_S_OperateOut operateOut;
				operateOut.wOperateCode = pData->wOperateCode;
				operateOut.wOperateUser = wChairID;
				operateOut.wProvideUser = it->second.dwActionProvider;
				operateOut.cbCardId = it->second.cbPlayerActionCardData;
				if(bCanQiangGangHu && (pData->wOperateCode & WIK_HU_MY))
				{
					operateOut.isQiangGang = 1;
				}
				else
				{
					operateOut.isQiangGang = 0;
				}


				for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
				{
					if(p == m_dwBanker)
					{
						SendTableData(p, SUB_S_OPERATE_OUT, &operateOut, sizeof(operateOut), true);
					}
					else
					{
						SendTableData(p, SUB_S_OPERATE_OUT, &operateOut, sizeof(operateOut), false);
					}
				}
			}

			SetMaxActionData(wChairID, pData->wOperateCode);

			//玩家操作完成,清除玩家的行为数据
			it->second.clearPlayerAction();

			//所有人操作完成进行下一步
			//CT_BOOL bIsHave = CT_FALSE;
			/*for(MapChairInfo::iterator itHave = m_mapChairInfo.begin(); itHave != m_mapChairInfo.end(); itHave++)
			{
				if(itHave->second.cbPlayerAction != WIK_NULL_MY)
				{
				    LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家ID: "<< itHave->second.dwUserID << " 有行为:"<<itHave->second.cbPlayerAction;
					bIsHave = CT_TRUE;
					break;
				}
			}*/

			//出牌后引发的行为,所有玩家都响应了
			m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_OUTCARD);

			//if(pData->wOperateCode == WIK_NULL_MY || pData->wOperateCode == WIK_NULL_MY_TIMEOUT)
			if (m_dwMaxAction == WIK_NULL_MY || m_dwMaxAction == WIK_NULL_MY_TIMEOUT)
			{
				if(bCanQiangGangHu)
				{
					//玩家能抢杠胡,但是点了过,就让巴杠玩家抓牌
					PlayerZhuaCard(Action_GangCard, m_dwCurGangPlayer);
				}
				else
				{
					//出牌引发的操作，能操作的玩家点了过,那么该玩家抓牌
					PlayerZhuaCard(Action_PassCard, wChairID);
				}
			}
			else
			{
				//当前二人麻将，所以作出行为的玩家就是最大行为玩家
				PlayerFinishAction(m_dwMaxActionPlayer, m_dwMaxAction);
			}
		}
	}
	else
	{
		LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 当前能作出的行为:"<< (int)it->second.cbPlayerAction
		<< " 行为描述: "<< CanActionStr(it->second.cbPlayerAction).c_str()
		<< " 是否完成行为:" << it->second.bFinishAction;
	}

	return CT_TRUE;
}

CT_VOID CGameProcess::SetMaxActionData(CT_DWORD dwMaxActionPlayer,  CT_DWORD dwMaxAction)
{
	//解决一家听 另外一家碰，杠 ，胡，吃后的行为错误问题
	/*if(m_dwMaxAction == WIK_NULL_MY && m_dwMaxActionPlayer == INVALID_CHAIR_ID)
	{
		m_dwMaxAction = dwMaxAction;
		m_dwMaxActionPlayer = dwMaxActionPlayer;
		//LOG(ERROR) << "设置最大行为玩家:" << m_pGameDeskPtr->GetUserID(m_dwMaxActionPlayer) << " 最大行为:" << m_dwMaxAction;
	}
	else
	{
		if(m_dwMaxAction == WIK_TING_MY)
		{
			m_dwMaxAction = dwMaxAction;
			m_dwMaxActionPlayer = dwMaxActionPlayer;
			//LOG(ERROR) << "设置最大行为玩家:" << m_pGameDeskPtr->GetUserID(m_dwMaxActionPlayer) << " 最大行为:" << m_dwMaxAction;
		}
	}*/

	m_dwMaxAction = dwMaxAction;
	m_dwMaxActionPlayer = dwMaxActionPlayer;
}
std::string CGameProcess::CanActionStr(CT_DWORD dwAction)
{
	std::string str;
	if((dwAction & WIK_CHI_MY) && (dwAction & WIK_CENTER_MY))
	{
		str = "中吃,";
	}
	if((dwAction & WIK_CHI_MY) && (dwAction & WIK_LEFT_MY))
	{
		str += "左吃,";
	}
	if((dwAction & WIK_CHI_MY) && (dwAction & WIK_RIGHT_MY))
	{
		str += "右吃,";
	}
	if((dwAction & WIK_PENG_MY))
	{
		str += "碰,";
	}
	if((dwAction & WIK_GANG_MY) && (dwAction & WIK_GANG_AN_MY))
	{
		str += "暗杠,";
	}
	if((dwAction & WIK_GANG_MY) && (dwAction & WIK_GANG_MING_MY))
	{
		str += "明杠,";
	}
	if((dwAction & WIK_GANG_MY) && (dwAction & WIK_GANG_MING_PENG_MY))
	{
		str += "巴杠,";
	}
	if((dwAction & WIK_HU_MY) && (dwAction & WIK_HU_ZI_MO_MY))
	{
		str += "自摸胡,";
	}
	if((dwAction & WIK_HU_MY) && (dwAction & WIK_HU_FANG_PAO_MY))
	{
		str += "放炮胡,";
	}

	if(str.empty())
	{
		return std::string("不知道操作是什么");
	} else{
		return str;
	}
}

CT_VOID CGameProcess::PlayerFinishAction(CT_DWORD dwMaxActionPlayer, CT_DWORD cbMaxAction)
{
	if(cbMaxAction & WIK_NULL_MY)
	{
		//二人麻将,过之后就是过的玩家抓牌
		//PlayerZhuaCard(Action_PassCard, dwMaxActionPlayer);
		LOG(ERROR) << "不应该到这里,dwMaxActionPlayer: " <<dwMaxActionPlayer << " cbMaxAction:" << cbMaxAction << " 玩家ID: "<<m_pGameDeskPtr->GetUserID(dwMaxActionPlayer);
	}
	else if(cbMaxAction & WIK_LEFT_MY || cbMaxAction & WIK_CENTER_MY || cbMaxAction & WIK_RIGHT_MY || cbMaxAction & WIK_PENG_MY || cbMaxAction & WIK_CHI_MY)
	{
		//二人麻将,吃碰之后就是吃碰的玩家打牌
		//玩家碰牌之后打牌默认为最右边那张
		CT_BOOL bFind = CT_FALSE;
		MapChairInfo::iterator it = m_mapChairInfo.find(dwMaxActionPlayer);
		if(it != m_mapChairInfo.end())
		{
			for(int i = MAX_INDEX - 1; i >= 0; i--)
			{
				if(it->second.cbHandCardIndex[i] > 0)
				{
                    CT_BYTE cbCardData = m_GameLogic.SwitchToCardData(i);

                    CT_BOOL bCanTing = CT_FALSE;
					//玩家吃 碰后，出牌前检查是否可以听牌
                    if(CanTing(dwMaxActionPlayer))
                    {
						bCanTing = CT_TRUE;
						m_eTingReason = TingTrigger::PengChiCard;
						TipPlayerAction(it->first, cbCardData, TipTrigger::Trigger_ZhuaCard);
                         /*m_eCanTingTrigger = Trigger_ZhuaCard;
                         m_cbCanTingTrigger_ActionCardData = cbCardData;

                         CMD_S_OperateTip_Send data;
                         data.wOperateCode = WIK_TING_MY;
                         data.bGangCount = 0;
                         memset(data.cbCardTB, 0, sizeof(data.cbCardTB));
                         m_dwCurOperatePlayer = dwMaxActionPlayer;
                         SendTableData(dwMaxActionPlayer, SUB_S_OPERATE_TIP_SEND, &data, sizeof(data), true);

                         if (cbMaxAction & WIK_CHI_MY)
                         {
                             LOG(ERROR) << "玩家: " << it->second.dwUserID << " 吃牌后,提示他听牌";
                         }

                         if(cbMaxAction & WIK_PENG_MY)
                         {
                             LOG(ERROR) << "玩家: " << it->second.dwUserID << " 碰牌后,提示他听牌";
                         }*/

						if(it->second.bIsTing == CT_FALSE && it->second.cbPlayerAction & WIK_TING_MY)
						{
							TipPlayerMax(it->first);
						}
					}

					if(bCanTing)
					{
						//如果吃碰能触发听牌,那么提示玩家出牌的时候就不要启动出牌定时器了。
						TipPlayerOutCard(dwMaxActionPlayer, cbCardData, CT_FALSE);
					}
					else
					{
						TipPlayerOutCard(dwMaxActionPlayer, cbCardData, CT_TRUE);
					}

					//玩家取消听牌，超时不出牌，系统自动出的牌
					m_cbCancelTing_OutTime_OutCard = cbCardData;
					bFind = CT_TRUE;
					break;
				}
			}
		}
		else
		{
			LOG(ERROR) << "玩家碰牌后设定超时打的牌时，通过椅子找不到玩家："<<dwMaxActionPlayer;
		}

		if(bFind == CT_FALSE)
		{
			LOG(ERROR) << "PlayerFinishAction dwMaxActionPlayer:"<<dwMaxActionPlayer << " cbMaxAction:" << cbMaxAction << " 没有调用TipPlayerOutCard";
		}
	}
	else if(cbMaxAction & WIK_GANG_MY || cbMaxAction & WIK_GANG_AN_MY || cbMaxAction & WIK_GANG_MING_MY || cbMaxAction & WIK_GANG_MING_PENG_MY)
	{
		//二人麻将,杠之后就是杠的玩家抓牌
		PlayerZhuaCard(Action_GangCard, dwMaxActionPlayer);
	}
	else if(cbMaxAction & WIK_HU_MY || cbMaxAction & WIK_HU_FANG_PAO_MY || cbMaxAction & WIK_HU_ZI_MO_MY)
	{
		//二人麻将，胡牌结束
		OnEventGameEnd(dwMaxActionPlayer,GER_NORMAL);
	}
	else if(cbMaxAction & WIK_TING_MY)
	{

	}
}

CT_BOOL CGameProcess::PlayerTingCard(CT_DWORD dwChairID)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "PlayerTingCard 通过椅子ID: "<< dwChairID << " 找不到玩家 ";
		return CT_FALSE;
	}

	if(it->second.bIsTing)
	{
		LOG(ERROR) << "PlayerTingCard 玩家ID：" << it->second.dwUserID << " 已经听牌";
		return CT_FALSE;
	}

	//标记已经听牌
	it->second.bIsTing = CT_TRUE;

	return CT_TRUE;
}

CT_BOOL CGameProcess::PlayerChiHuCard(CT_DWORD dwChiHuPlayer, CT_BYTE cbChiChuCard, CT_BOOL bZiMo)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChiHuPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家ID: " << m_pGameDeskPtr->GetUserID(dwChiHuPlayer) << " 胡牌时,在椅子上找不到玩家" << " 要胡的牌是:" << (int)cbChiChuCard;
		return CT_FALSE;
	}

	//
	it->second.cbHuCard = cbChiChuCard;
	it->second.bHu = CT_TRUE;
	it->second.bZiMo = bZiMo;

	//因为把抢杠胡当成的放炮胡，所以在抢杠胡的时候不能在巴杠玩家的出牌区中删除出的牌
	if(bZiMo == CT_FALSE)
    {
		if(!it->second.bCanQiangGangHu)
		{
			MapChairInfo::iterator itDelDiscard = m_mapChairInfo.find(m_dwCurFinishOutCardPlayer);
			if(itDelDiscard != m_mapChairInfo.end())
			{
			    itDelDiscard->second.cbDiscardCard[itDelDiscard->second.cbDiscardCount-1] = INVALID_CARD_DATA;
			    itDelDiscard->second.cbDiscardCount--;

				//这里要改
				/*int nLoop = itDelDiscard->second.cbDiscardCount > MAX_OUT_NUM ? MAX_OUT_NUM : itDelDiscard->second.cbDiscardCount;
				for(int i = 0 ; i < nLoop; i++)
				{
					if(itDelDiscard->second.cbDiscardCard[i] == cbChiChuCard)
					{
						itDelDiscard->second.cbDiscardCard[i] = INVALID_CARD_DATA;
						itDelDiscard->second.cbDiscardCount--;
						break;
					}
				}*/
			}
			else
			{
				LOG(ERROR) << "放炮胡后，删除弃牌的时候，通过椅子ID:"<<m_dwCurFinishOutCardPlayer<< " 找不到玩家";
			}
		}
		else
		{
			//从巴杠玩家把杠改成碰
			MapChairInfo::iterator itBaGangPlayer = m_mapChairInfo.find(m_dwCurGangPlayer);
			if(itBaGangPlayer == m_mapChairInfo.end())
			{
				LOG(ERROR) << "通过椅子ID: "<<m_dwCurGangPlayer << " 找不到玩家";
			}
			else
			{
				for(int i = 0; i < MAX_WEAVE; i++)
				{
					if(itBaGangPlayer->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_PENG_MY
					&& itBaGangPlayer->second.weaveItem[i].cbCenterCard == cbChiChuCard)
					{
						itBaGangPlayer->second.weaveItem[i].cbWeaveKind = WIK_PENG_MY;
						itBaGangPlayer->second.weaveItem[i].cbCardData[3] = INVALID_CARD_DATA;
						break;
					}
				}

			}
		}
    }
	return CT_TRUE;
}

CT_BOOL CGameProcess::PlayerChiCard(CT_DWORD dwChiCardPlayer, CT_BYTE cbChiCardData, CT_DWORD dwChiType)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChiCardPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家ID: " << m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 吃牌时,在椅子上找不到玩家" << " 要吃的牌是:" << (int)cbChiCardData;
		return CT_FALSE;
	}

	CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbChiCardData);
	if(cbCardColor != Color_WAN)
	{
		LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 要吃的牌:"<<(int)cbChiCardData << " 花色错误";
		return CT_FALSE;
	}

	if(dwChiType & WIK_LEFT_MY)
	{
		if(cbChiCardData > 0x07)
		{
			LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 左吃错误,吃的牌: "<<(int)cbChiCardData;
			return CT_FALSE;
		}

		CT_BYTE cbCardData1 = cbChiCardData + 1, cbCardData2 = cbChiCardData + 2;
		//判断是否有能吃的牌
		if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData1) == CT_FALSE || m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData2) == CT_FALSE)
		{
			LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(dwChiCardPlayer)<<" 左吃的牌:"<<(int)cbChiCardData << " 但是没有牌:" <<(int)cbCardData1 << " 和 "<<(int)cbCardData2;
			return CT_FALSE;
		}

		//移除手牌
		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData1))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌2: "<< (int)cbCardData1 << " 失败";
        }
		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData2))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌3: "<< (int)cbCardData2 << " 失败";
        }

		//加入组合牌
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
			{
				continue;
			}

			it->second.weaveItem[i].cbWeaveKind = WIK_LEFT_MY;
			it->second.weaveItem[i].cbCenterCard = cbChiCardData;
			it->second.weaveItem[i].cbPublicCard = CT_TRUE;
			it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
			it->second.weaveItem[i].cbCardData[0] = cbChiCardData;
			it->second.weaveItem[i].cbCardData[1] = cbCardData1;
			it->second.weaveItem[i].cbCardData[2] = cbCardData2;
			break;
		}
	}
	else if(dwChiType & WIK_CENTER_MY)
	{
		if(cbChiCardData == 0x09 || cbChiCardData == 0x01)
		{
			LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 中吃的牌: "<< (int)cbChiCardData << " 错误";
			return CT_FALSE;
		}

		CT_BYTE cbCardData1 = cbChiCardData - 1, cbCardData2 = cbChiCardData + 1;
		//判断是否有能吃的牌
		if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData1) == CT_FALSE || m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData2) == CT_FALSE)
		{
			LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 中吃的牌:" <<(int)cbChiCardData << " 但是没有牌:"<<(int)cbCardData1 << " 和 "<<(int)cbCardData2;
			return CT_FALSE;
		}

		//移除手牌
		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData1))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌5: "<< (int)cbCardData1 << " 失败";
        }
		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData2))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌6: "<< (int)cbCardData2 << " 失败";
        }

		//加入组合牌
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
			{
				continue;
			}

			it->second.weaveItem[i].cbWeaveKind = WIK_CENTER_MY;
			it->second.weaveItem[i].cbCenterCard = cbChiCardData;
			it->second.weaveItem[i].cbPublicCard = CT_TRUE;
			it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
			it->second.weaveItem[i].cbCardData[0] = cbCardData1;
			it->second.weaveItem[i].cbCardData[1] = cbChiCardData;
			it->second.weaveItem[i].cbCardData[2] = cbCardData2;
			break;
		}
	}
	else if(dwChiType & WIK_RIGHT_MY)
	{
		if(cbChiCardData <= 0x02)
		{
			LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 右吃的牌: "<< (int)cbChiCardData << " 错误";
			return CT_FALSE;
		}

		CT_BYTE cbCardData1 = cbChiCardData - 1, cbCardData2 = cbChiCardData - 2;
		//判断是否有能吃的牌
		if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData1) == CT_FALSE || m_GameLogic.CheckCard(it->second.cbHandCardIndex, cbCardData2) == CT_FALSE)
		{
			LOG(ERROR) << "玩家ID: "<< m_pGameDeskPtr->GetUserID(dwChiCardPlayer) << " 右吃的牌:" <<(int)cbChiCardData << " 但是没有牌:"<<(int)cbCardData1 << " 和 "<<(int)cbCardData2;
			return CT_FALSE;
		}

		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData1))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌8: "<< (int)cbCardData1 << " 失败";
        }
		if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbCardData2))
        {
            LOG(ERROR) << "从玩家ID:" << it->second.dwUserID << " 身上移除授牌9: "<< (int)cbCardData2 << " 失败";
        }

		//加入组合牌
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
			{
				continue;
			}

			it->second.weaveItem[i].cbWeaveKind = WIK_RIGHT_MY;
			it->second.weaveItem[i].cbCenterCard = cbChiCardData;
			it->second.weaveItem[i].cbPublicCard = CT_TRUE;
			it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
			it->second.weaveItem[i].cbCardData[0] = cbCardData1;
			it->second.weaveItem[i].cbCardData[1] = cbCardData2;
			it->second.weaveItem[i].cbCardData[2] = cbChiCardData;
			break;
		}
	}

    MapChairInfo::iterator itDelDiscard = m_mapChairInfo.find(m_dwCurFinishOutCardPlayer);
    if(itDelDiscard != m_mapChairInfo.end())
    {
        itDelDiscard->second.cbDiscardCard[itDelDiscard->second.cbDiscardCount-1] = INVALID_CARD_DATA;
        itDelDiscard->second.cbDiscardCount--;

        /*int nLoop = itDelDiscard->second.cbDiscardCount > MAX_OUT_NUM ? MAX_OUT_NUM : itDelDiscard->second.cbDiscardCount;
        for(int i = 0 ; i < nLoop; i++)
        {
			//这里要改
            if(itDelDiscard->second.cbDiscardCard[i] == cbChiCardData)
            {
                itDelDiscard->second.cbDiscardCard[i] = INVALID_CARD_DATA;
                itDelDiscard->second.cbDiscardCount--;
                break;
            }
        }*/
    }
    else
    {
        LOG(ERROR) << "吃牌后，删除弃牌的时候，通过椅子ID:"<<m_dwCurFinishOutCardPlayer<< " 找不到玩家";
    }

	if(m_cbCurOutCardData == cbChiCardData)
	{
		m_cbCurOutCardData = INVALID_CARD_DATA;
	}

	return CT_TRUE;
}

CT_BOOL CGameProcess::PlayerPengCard(CT_DWORD dwPengPaiPlayer, CT_BYTE cbPengCardData)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwPengPaiPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家ID: " << m_pGameDeskPtr->GetUserID(dwPengPaiPlayer) << " 碰牌时,在椅子上找不到玩家" << " 要碰的牌是:" << (int)cbPengCardData;
		return CT_FALSE;
	}

	CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbPengCardData);
	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(it->second.cbHandCardIndex, cbCardIndex);
	if(cbCardCount < 2)
	{
		LOG(ERROR) << "玩家ID: "<<m_pGameDeskPtr->GetUserID(dwPengPaiPlayer) << " 碰的牌是: "<<(int)cbPengCardData << " 但是手中只有:"<< (int)cbCardCount<<" 张";
		return CT_FALSE;
	}

	if(m_cbCurOutCardData == cbPengCardData)
	{
		m_cbCurOutCardData = INVALID_CARD_DATA;
	}
	//移除手牌
	for(int i = 0 ;i < 2 ;i++)
	{
		if(m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbPengCardData) == CT_FALSE)
		{
			LOG(ERROR) << "严重错误:" << "玩家ID: "<<m_pGameDeskPtr->GetUserID(dwPengPaiPlayer) << " 碰牌的时候，移除牌:" <<(int)cbPengCardData << " 失败";
		}
	}

	//碰的牌加入到组合牌中
	for(int i = 0; i < MAX_WEAVE; i++)
	{
		if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
		{
			continue;
		}

		it->second.weaveItem[i].cbWeaveKind = WIK_PENG_MY;
		it->second.weaveItem[i].cbCenterCard = cbPengCardData;
		it->second.weaveItem[i].cbPublicCard = CT_TRUE;
		it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
		it->second.weaveItem[i].cbCardData[0] = cbPengCardData;
		it->second.weaveItem[i].cbCardData[1] = cbPengCardData;
		it->second.weaveItem[i].cbCardData[2] = cbPengCardData;
		break;
	}

    MapChairInfo::iterator itDelDiscard = m_mapChairInfo.find(m_dwCurFinishOutCardPlayer);
    if(itDelDiscard != m_mapChairInfo.end())
    {
        itDelDiscard->second.cbDiscardCard[itDelDiscard->second.cbDiscardCount-1] = INVALID_CARD_DATA;
        itDelDiscard->second.cbDiscardCount--;

		//这里要改
        /*int nLoop = itDelDiscard->second.cbDiscardCount > MAX_OUT_NUM ? MAX_OUT_NUM : itDelDiscard->second.cbDiscardCount;
        for(int i = 0 ; i < nLoop; i++)
        {
            if(itDelDiscard->second.cbDiscardCard[i] == cbPengCardData)
            {
                itDelDiscard->second.cbDiscardCard[i] = INVALID_CARD_DATA;
                itDelDiscard->second.cbDiscardCount--;
                break;
            }
        }*/
    }
    else
    {
        LOG(ERROR) << "碰牌后，删除弃牌的时候，通过椅子ID:"<<m_dwCurFinishOutCardPlayer<< " 找不到玩家";
    }

	if(m_cbCurOutCardData == cbPengCardData)
	{
		m_cbCurOutCardData = INVALID_CARD_DATA;
	}

    return CT_TRUE;
}

CT_BOOL CGameProcess::PlayerGangCard(CT_DWORD dWGangCardPlayer, CT_BYTE cbGangCardData, CT_DWORD dwGangType)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dWGangCardPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家ID: " << m_pGameDeskPtr->GetUserID(dWGangCardPlayer) << " 杠牌时,在椅子上找不到玩家" << " 要杠的牌是:" << (int)cbGangCardData;
		return CT_FALSE;
	}

	if(dwGangType & WIK_GANG_MY)
	{
		if(dwGangType & WIK_GANG_MING_MY)
		{
			//删除手牌
			for(int i = 0; i < 3; i++)
			{
				if(m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbGangCardData) == CT_FALSE)
				{
					LOG(ERROR) << "严重错误, 玩家:" <<it->second.dwUserID <<" 明杠时从手牌中删除牌：" << (int)cbGangCardData << " 失败";
				}
			}

			//加入到组合牌中
			for(int i = 0; i < MAX_WEAVE; i++)
			{
				if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
				{
					continue;
				}

				it->second.weaveItem[i].cbWeaveKind = WIK_GANG_MING_MY;
				it->second.weaveItem[i].cbCenterCard = cbGangCardData;
				it->second.weaveItem[i].cbPublicCard = CT_TRUE;
				it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
				it->second.weaveItem[i].cbCardData[0] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[1] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[2] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[3] = cbGangCardData;
				break;
			}
		}
		else if(dwGangType & WIK_GANG_AN_MY)
		{
			//删除手牌
			for(int i = 0; i < 4; i++)
			{
				if(m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbGangCardData) == CT_FALSE)
				{
					LOG(ERROR) << "严重错误, 玩家: "<< it->second.dwUserID <<" 暗杠时从手牌中删除牌：" << (int)cbGangCardData << " 失败";
				}
			}

			//加入到组合牌中
			for(int i = 0; i < MAX_WEAVE; i++)
			{
				if(it->second.weaveItem[i].cbWeaveKind != WIK_NULL_MY)
				{
					continue;
				}

				it->second.weaveItem[i].cbWeaveKind = WIK_GANG_AN_MY;
				it->second.weaveItem[i].cbCenterCard = cbGangCardData;
				it->second.weaveItem[i].cbPublicCard = CT_FALSE;
				it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
				it->second.weaveItem[i].cbCardData[0] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[1] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[2] = cbGangCardData;
				it->second.weaveItem[i].cbCardData[3] = cbGangCardData;
				break;
			}
		}
		else if(dwGangType & WIK_GANG_MING_PENG_MY)
		{
			if(m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbGangCardData) == CT_FALSE)
			{
				LOG(ERROR) << "严重错误, 玩家巴杠时从手牌中删除牌：" << (int)cbGangCardData << " 失败";
			}

			CT_BOOL bBaGangSucc = CT_FALSE;
			for(int i = 0; i < MAX_WEAVE; i++)
			{
				if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY && it->second.weaveItem[i].cbCenterCard == cbGangCardData)
				{
					bBaGangSucc = CT_TRUE;
					it->second.weaveItem[i].cbWeaveKind = WIK_GANG_MING_PENG_MY;
					it->second.weaveItem[i].cbCenterCard = cbGangCardData;
					it->second.weaveItem[i].cbPublicCard = CT_TRUE;
					it->second.weaveItem[i].wProvideUser = it->second.dwActionProvider;
					it->second.weaveItem[i].cbCardData[0] = cbGangCardData;
					it->second.weaveItem[i].cbCardData[1] = cbGangCardData;
					it->second.weaveItem[i].cbCardData[2] = cbGangCardData;
					it->second.weaveItem[i].cbCardData[3] = cbGangCardData;
					break;
				}
			}

			if(bBaGangSucc == CT_FALSE)
			{
				LOG(ERROR) << "玩家ID:" << m_pGameDeskPtr->GetUserID(dWGangCardPlayer) << " 巴杠失败";
				return CT_FALSE;
			}
		}
	}
	else
	{
		LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << " 要杠牌，但是杠牌类型错误:"<< dwGangType << " 要杠的牌是: "<< (int)cbGangCardData;
	}

    if(dwGangType & WIK_GANG_MING_MY)
    {
        MapChairInfo::iterator itDelDiscard = m_mapChairInfo.find(m_dwCurFinishOutCardPlayer);
        if(itDelDiscard != m_mapChairInfo.end())
        {
            itDelDiscard->second.cbDiscardCard[itDelDiscard->second.cbDiscardCount-1] = INVALID_CARD_DATA;
            itDelDiscard->second.cbDiscardCount--;

			//这里要改
           /* int nLoop = itDelDiscard->second.cbDiscardCount > MAX_OUT_NUM ? MAX_OUT_NUM : itDelDiscard->second.cbDiscardCount;
            for(int i = 0 ; i < nLoop; i++)
            {
                if(itDelDiscard->second.cbDiscardCard[i] == cbGangCardData)
                {
                    itDelDiscard->second.cbDiscardCard[i] = INVALID_CARD_DATA;
                    itDelDiscard->second.cbDiscardCount--;
                    break;
                }
            }*/
        }
        else
        {
            LOG(ERROR) << "明杠后，删除弃牌的时候，通过椅子ID:"<<m_dwCurFinishOutCardPlayer<< " 找不到玩家";
        }
    }

	m_dwCurGangPlayer = dWGangCardPlayer;
	return CT_TRUE;
}

CT_BOOL CGameProcess::OnMsg_PlayerOutCard(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	//有玩家出牌了就删除自动出牌定时器
	//m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);

	if(dwDataSize != sizeof(CMD_C_OutCard))
	{
		LOG(ERROR) << "dwDataSize:" << dwDataSize << " is not equal to:" << sizeof(CMD_C_OutCard);
		return CT_TRUE;
	}

	//玩家是否还在线
	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "出牌时通过椅子ID:" << wChairID << " 找不到玩家了";
		return CT_TRUE;
	}

	//是否该他出牌
	if(wChairID != m_dwShouldOutCardPlayer)
	{
		LOG(ERROR) << "当前出牌玩家为: "<< m_pGameDeskPtr->GetUserID(wChairID) << " 实际上应该是:" << m_pGameDeskPtr->GetUserID(m_dwShouldOutCardPlayer);
		return CT_TRUE;
	}

	CMD_C_OutCard *pOutCardData = (CMD_C_OutCard *)pDataBuffer;
	//这张牌是否有效
	if(m_GameLogic.IsValidCard(pOutCardData->cbCardId) == CT_FALSE)
	{
		LOG(ERROR) << "玩家: "<< m_pGameDeskPtr->GetUserID(wChairID) << " 出的牌: " << (int)pOutCardData->cbCardId << " 无效";
		return CT_TRUE;
	}

	//出牌者是否有这张牌
	if(m_GameLogic.CheckCard(it->second.cbHandCardIndex, pOutCardData->cbCardId) == CT_FALSE)
	{
		LOG(ERROR) << "出牌时玩家: " << m_pGameDeskPtr->GetUserID(wChairID) << " 没有牌:"<< (int)pOutCardData->cbCardId;
		return CT_TRUE;
	}

	//是否连续几次出牌
	if(m_bCurOutCardPlayerFinish)
	{
		LOG(ERROR) << "玩家: "<<m_pGameDeskPtr->GetUserID(wChairID) << " 之前已经完成了出牌，不能连续出牌，出牌失败,想出的牌是：" <<(int)pOutCardData->cbCardId;
		return CT_TRUE;
	}

	//如果玩家听牌了,那么就不能手动出牌了
	if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing == CT_FALSE)
    {
	    LOG(ERROR) << "玩家: "<< it->second.dwUserID << " 听牌后已经主动出过一张牌了，后面的牌都是自动出牌，但是客户端却发送了出牌消息";
	    return CT_TRUE;
    }

	PlayerOutCard(wChairID, pOutCardData->cbCardId, pOutCardData->cbCardIndex, CT_FALSE);

	return CT_TRUE;
}

CT_VOID CGameProcess::PlayerOutCard(CT_WORD wChairID, CT_BYTE cbOutCardData, CT_BYTE cbOutCardIndex, CT_BOOL bAutoOutCard)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(wChairID);
	if(it == m_mapChairInfo.end())
	{
		return;
	}

	if(m_cbCurState != STATE_WAIT_OUT && m_cbCurState != STATE_WAIT_SEND_OPERATE)
	{
		LOG(ERROR) << "玩家:" << it->second.dwUserID << " 出牌时,当前的牌局状态不是出牌状态,m_cbCurState:" <<(int)m_cbCurState;
		return;
	}

	if(m_dwCurOperatePlayer != wChairID)
	{
		LOG(ERROR) << "玩家:" <<it->second.dwUserID << " 发起出牌: "<< (int)cbOutCardData << " 但是目前应该是玩家: "<< m_pGameDeskPtr->GetUserID(m_dwCurOperatePlayer)
		<< " 出牌" << " 是否自动出牌: "<< bAutoOutCard;
		return;
	}

    if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing)
    {
        CT_BOOL bFind = CT_FALSE;
        for(CT_BYTE i = 0; i < MAX_CARD_HAND; i++)
        {
            if(it->second.cbMustOutCard_AfterClickTing[i] == cbOutCardData)
            {
                bFind = CT_TRUE;
                break;
            }
        }
        if(bFind == CT_FALSE)
        {
            //听后出的牌不能导致玩家胡牌
            LOG(ERROR) << "玩家ID: "<<it->second.dwUserID << " 出的牌是"<<(int)cbOutCardData << "但是出了这张牌不能胡牌";
            return;
        }
    }

	if( CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbOutCardData))
    {
	    LOG(ERROR) << "玩家ID: "<< it->second.dwUserID << " 出牌的时候, 出的牌是: " << (int)cbOutCardData << " 但是从手牌中移除失败";
    }

	CMD_S_OperateTip_Cancel cancel;
	cancel.wOperateCode = WIK_NULL_MY;
	SendTableData(wChairID, SUB_S_OPERATE_TIP_CANCEL, &cancel, sizeof(cancel), true);

	//发送玩家出牌消息
	CMD_S_OutCard outCard;
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		if(it->first == wChairID)
		{
			outCard.wOutCardUser = wChairID;
			outCard.cbCardId = cbOutCardData;
			outCard.cbCardIndex = cbOutCardIndex;
			outCard.bIsAutoOutCard = bAutoOutCard;
            m_pGameDeskPtr->SendTableData(it->first,SUB_S_OUT_CARD,&outCard,sizeof(outCard), true);
		}
		else
		{
			outCard.wOutCardUser = wChairID;
			outCard.cbCardId = cbOutCardData;
			outCard.cbCardIndex = 0;
			outCard.bIsAutoOutCard = bAutoOutCard;
            m_pGameDeskPtr->SendTableData(it->first,SUB_S_OUT_CARD,&outCard,sizeof(outCard), false);
		}
	}

	LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:"<< it->second.dwUserID << " 出牌:"<<(int)cbOutCardData;
    m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
    /*if(m_cbCurState == STATE_WAIT_SEND_OPERATE)
    {*/
        //如果玩家抓牌触发行为,玩家直接出牌了,那么要记得删除定时器IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD
        m_pGameDeskPtr->KillGameTimer(IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD);
    //}
    m_dwCurFinishOutCardPlayer = wChairID;
	//玩家当前要进行的操作是加倍
	m_cbCurState = STATE_DEFAUT;
	//记录玩家出牌信息
	it->second.cbDiscardCard[it->second.cbDiscardCount] = cbOutCardData;
	it->second.cbDiscardCount++;
	it->second.cbOutCardCount++;
	it->second.cbPlayerLastOutCardData = cbOutCardData;
	m_cbCurOutCardData = cbOutCardData;
	m_bCurOutCardPlayerFinish = CT_TRUE;
    m_dwCurOutCardPlayer = wChairID;
	m_cbAllPlayerOutCardCount++;
	m_bGangStatus = CT_FALSE;
    m_cbLastZhuaCardData = INVALID_CARD_DATA;
    if(wChairID == m_dwBanker && m_bZhuangFirstOutCard)
	{
		m_bZhuangFirstOutCard = CT_FALSE;
	}
	if(it->second.bIsTing && it->second.bIsOutFirstCard_AfterTing)
	{
		it->second.bIsOutFirstCard_AfterTing = CT_FALSE;
		m_dwTingCardPlayerCnt++;

		//玩家点击听按钮后,出了第一张牌,就广播听牌
		CMD_S_OperateSend tip;
		tip.wOperateCode = WIK_TING_MY;
		tip.wOperateUser = it->first;
		tip.cbCardId = INVALID_CARD_DATA;

		for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
		{
			if(p == m_dwBanker)
			{
				SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), true);
			}
			else
			{
				SendTableData(p, SUB_S_OPERATE_SEND, &tip, sizeof(tip), false);
			}
		}

		//点击听按钮后并且出第一张牌后才算真正的听牌
		if(it->first == m_dwBanker && m_bCanTianTingBanker)
		{
			//庄家在能天听的情况下，选择了听
			it->second.bTianTing = CT_TRUE;
			m_bCanTianTingBanker = CT_FALSE;
		}

		if(it->first != m_dwBanker && m_bCanTianTingXian)
		{
			it->second.bTianTing = CT_TRUE;
			m_bCanTianTingXian = CT_FALSE;
		}

		{
			CMD_S_ShowCard_Ting ting;
			if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
			{
				ting.isShowCard = 1;
			}
			else
			{
				ting.isShowCard = 0;
			}

			if(ting.isShowCard == 0)
			{
				ting.wOperateUser = it->first;
				memset(ting.bCurHandCardData, INVALID_CARD_DATA, sizeof(ting.bCurHandCardData));
				m_GameLogic.SwitchToCardData(it->second.cbHandCardIndex, ting.bCurHandCardData);
				for (CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
				{
					if (p == m_dwBanker)
					{
						SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), true);
					}
					else
					{
						SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), false);
					}
				}
			}
			else
			{
				for(MapChairInfo::iterator itTing = m_mapChairInfo.begin(); itTing != m_mapChairInfo.end(); itTing++)
				{
                    ting.wOperateUser = itTing->first;
                    memset(ting.bCurHandCardData, INVALID_CARD_DATA, sizeof(ting.bCurHandCardData));
                    m_GameLogic.SwitchToCardData(itTing->second.cbHandCardIndex, ting.bCurHandCardData);

                    for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
                    {
                        if(p == m_dwBanker)
                        {
                            SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), true);
                        }
                        else
                        {
                            SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), false);
                        }
                    }
				}
			}
		}

		if(m_dwTingCardPlayerCnt >= GAME_PLAYER)
		{
		    for(MapChairInfo::iterator itTing = m_mapChairInfo.begin(); itTing != m_mapChairInfo.end(); itTing++)
            {
                /*CMD_S_ShowCard_Ting ting;
                ting.wOperateUser = itTing->first;
                memset(ting.bCurHandCardData, INVALID_CARD_DATA, sizeof(ting.bCurHandCardData));
                m_GameLogic.SwitchToCardData(itTing->second.cbHandCardIndex, ting.bCurHandCardData);

                for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
                {
                    if(p == m_dwBanker)
                    {
                        SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), true);
                    }
                    else
                    {
                        SendTableData(p, SUB_S_SHOWCARD_TING, &ting, sizeof(ting), false);
                    }
                }*/

                if(itTing->first == wChairID)
				{
					TipPlayerMayBeHuCard_AfterTing(itTing->first, itTing->second.cbPlayerLastOutCardData, CT_FALSE, CT_TRUE);
				}
				else
				{
					TipPlayerMayBeHuCard_AfterTing(itTing->first, itTing->second.cbPlayerLastOutCardData, CT_FALSE, CT_FALSE);
				}
            }
		}
		else
        {
            TipPlayerMayBeHuCard_AfterTing(wChairID, cbOutCardData, CT_FALSE, CT_TRUE, CT_FALSE);
        }
	}

	if(m_bCanTianTingBanker && wChairID == m_dwBanker && it->second.bIsTing == CT_FALSE)
	{
		//如果庄家能天听,但是却没有点听按钮,就直接出牌了，那么就放弃天听了。
		it->second.bTianTing = CT_FALSE;
		m_bCanTianTingBanker = CT_FALSE;
	}

	if(m_bCanTianTingXian && wChairID != m_dwBanker && it->second.bIsTing == CT_FALSE)
	{
		it->second.bTianTing = CT_FALSE;
		m_bCanTianTingXian = CT_FALSE;
	}

	ClearMaxActionData();
	//判断其他玩家是否有行为
	CT_BOOL bHaveActionAfterOutCard = IsHaveAction_AfterOutCard(wChairID, cbOutCardData);
	if(bHaveActionAfterOutCard)
	{
		m_cbCurState = STATE_WAIT_OUT_OPERATE;
		m_startOperateTime = Utility::GetTick();
		//除了出牌的玩家其他有行为的玩家都会得到通知
		TipPlayerAction(wChairID, cbOutCardData, TipTrigger::Trigger_OutCard);
	}
	else
	{
		//抓牌 弄完了看看IsValidCard接口，然后就可以测试了
		PlayerZhuaCard(Action_OutCard, wChairID);
	}
}

CT_VOID CGameProcess::TipPlayerAction(CT_DWORD dwExpectPlayer, CT_BYTE cbActionCardData, TipTrigger tipTrigger, CT_BOOL bStartOperateTimeOutTimer /*= CT_TRUE*/)
{
	if(tipTrigger == Trigger_OutCard)
	{
		//玩家出牌导致的玩家行为提示
		for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
		{
		    if(dwExpectPlayer == it->first)
			{
				continue;
			}

			if(it->second.cbPlayerAction != WIK_NULL_MY)
			{
			    if(it->second.cbPlayerAction & WIK_JIABEI)
                {
					//LOG_IF(WARNING, PRINT_LOG_INFO) <<"提示玩家:"<<it->second.dwUserID << " 加倍";
					//玩家当前要进行的操作是加倍
					m_cbCurState = STATE_JIABEI;
					m_startOperateTime = Utility::GetTick();
			    	//别人出牌引发加倍
			    	it->second.cbJiaBeiSource = 2;
			        //听牌的玩家如果能胡的话 要提示胡 和 加倍
                    CMD_S_OperateTip_Ting ting;
                    ting.wOperateCode = it->second.cbPlayerAction;
					m_dwCurOperatePlayer = it->first;
                    SendTableData(it->first, SUB_S_OPERATE_TING, &ting, sizeof(ting), true);

                    if(bStartOperateTimeOutTimer)
                    {
						m_pGameDeskPtr->SetGameTimer(IDI_AUTO_HU_CARD_AFTER_JIABEI,
													 TIMER_AUTO_HU_CARD_AFTER_JIABEI * 1000, it->first);
					}
                }
                else
                {
					m_cbCurState = STATE_WAIT_OUT_OPERATE;
					m_dwCurOperatePlayer = it->first;
                    CMD_S_OperateTip tip;
                    tip.cbCardId = cbActionCardData;
                    tip.wOperateCode = it->second.cbPlayerAction;
                    SendTableData(it->first, SUB_S_OPERATE_TIP_OUT, &tip, sizeof(tip), true);

                    if(bStartOperateTimeOutTimer)
                    {
						//启动定时器，如果玩家不做行为的话，就是过
						m_pGameDeskPtr->SetGameTimer(IDI_OPERATE_TIME_OUT_AFTER_OUTCARD, TIMER_OPERATE_TIME_OUT * 1000, it->first);
					}
                }

                if(it->second.cbPlayerAction & WIK_JIABEI)
				{
                	LOG_IF(WARNING, PRINT_LOG_INFO) <<"提示玩家:"<<it->second.dwUserID<<" 加倍";
				}
                if(it->second.cbPlayerAction & WIK_TING_MY)
                {
                	//玩家能听牌记录玩家听牌的触发器
					m_eCanTingTrigger = Trigger_OutCard;
					m_cbCanTingTrigger_ActionCardData = cbActionCardData;
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 听牌:";
                }
				if((it->second.cbPlayerAction & WIK_CENTER_MY) && (it->second.cbPlayerAction & WIK_CHI_MY))
                {
				    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 中吃牌:" << (int)cbActionCardData;
                }
                if((it->second.cbPlayerAction & WIK_LEFT_MY) && (it->second.cbPlayerAction & WIK_CHI_MY))
                {
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 左吃牌:" << (int)cbActionCardData;
                }
                if((it->second.cbPlayerAction & WIK_RIGHT_MY) && (it->second.cbPlayerAction & WIK_CHI_MY))
                {
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 右吃牌:" << (int)cbActionCardData;
                }
                if(it->second.cbPlayerAction & WIK_PENG_MY)
                {
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 碰牌:" << (int)cbActionCardData;
                }
                if((it->second.cbPlayerAction & WIK_GANG_MING_MY) && (it->second.cbPlayerAction & WIK_GANG_MY))
                {
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 明杠牌:" << (int)cbActionCardData;
                }
                if((it->second.cbPlayerAction & WIK_HU_FANG_PAO_MY) && (it->second.cbPlayerAction & WIK_HU_MY))
                {
                    LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 放炮胡牌:" << (int)cbActionCardData;
                    CT_BYTE cbHandCardData[MAX_HAND_CARD_COUNT];
                    memset(cbHandCardData, INVALID_CARD_DATA, sizeof(cbHandCardData));
                    m_GameLogic.SwitchToCardData(it->second.cbHandCardIndex, cbHandCardData);
                    //LOG_IF(WARNING, PRINT_LOG_INFO) <<"玩家:" << it->second.dwUserID << " 此时的手牌数据为: ";
                    //for(int m = 0; m < MAX_HAND_CARD_COUNT; m++)
					//{
                    	//if(cbHandCardData[m] != INVALID_CARD_DATA)
						//{
                    		//LOG_IF(WARNING, PRINT_LOG_INFO) << (int)cbHandCardData[m] << "  ";
						//}
					//}
                }
			}
		}
	}
	else if(tipTrigger == Trigger_ZhuaCard)
	{
		//玩家抓牌导致的玩家行为提示
		MapChairInfo::iterator it = m_mapChairInfo.find(dwExpectPlayer);
		if(it != m_mapChairInfo.end())
		{
			if(it->second.cbPlayerAction != WIK_NULL_MY)
			{
				if(it->second.cbPlayerAction & WIK_JIABEI)
				{
					//玩家当前要进行的操作是加倍
					m_cbCurState = STATE_JIABEI;
					m_startOperateTime = Utility::GetTick();
					//自己抓牌引发加倍
					it->second.cbJiaBeiSource = 1;
					LOG_IF(WARNING, PRINT_LOG_INFO) <<"提示玩家:"<<it->second.dwUserID << " 加倍";
					CMD_S_OperateTip_Ting ting;
					ting.wOperateCode = it->second.cbPlayerAction;
					m_dwCurOperatePlayer = it->first;
					SendTableData(it->first, SUB_S_OPERATE_TING, &ting, sizeof(ting), true);

					if(bStartOperateTimeOutTimer)
					{
						m_pGameDeskPtr->SetGameTimer(IDI_AUTO_HU_CARD_AFTER_JIABEI,
													 TIMER_AUTO_HU_CARD_AFTER_JIABEI * 1000, it->first);
					}
				}
				else
				{
					CMD_S_OperateTip_Send tip;
					tip.wOperateCode = it->second.cbPlayerAction;

					if(it->second.cbPlayerAction & WIK_HU_MY)
					{
						tip.bGangCount = 0;
					}

					if(it->second.cbPlayerAction & WIK_GANG_MY)
					{
						tip.bGangCount = it->second.canGangCard.cbCardCount;
						int nLoop = tip.bGangCount > GANG_CARD_MAX ? GANG_CARD_MAX : tip.bGangCount;
						for(int i = 0; i < nLoop; i++)
						{
							tip.cbCardTB[i].cbCardId = it->second.canGangCard.cbCardData[i];
							tip.cbCardTB[i].wGangOperateCode = it->second.canGangCard.dwGangCardType[i];
						}
					}

					if((tip.wOperateCode & WIK_HU_MY)  && (tip.wOperateCode & WIK_HU_ZI_MO_MY))
					{
						LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 自摸胡,牌:" << (int)cbActionCardData;

						CT_BYTE cbHandCardData[MAX_HAND_CARD_COUNT];
						memset(cbHandCardData, INVALID_CARD_DATA, sizeof(cbHandCardData));
						m_GameLogic.SwitchToCardData(it->second.cbHandCardIndex, cbHandCardData);
						//LOG_IF(WARNING, PRINT_LOG_INFO) <<"玩家:" << it->second.dwUserID << " 此时的手牌数据为: ";
						//for(int m = 0; m < MAX_HAND_CARD_COUNT; m++)
						//{
							//if(cbHandCardData[m] != INVALID_CARD_DATA)
							//{
								//LOG_IF(WARNING, PRINT_LOG_INFO) << (int)cbHandCardData[m] << "  ";
							//}
						//}
					}
					if((tip.bGangCount > 0) && (tip.wOperateCode & WIK_GANG_MY) && (tip.wOperateCode & WIK_GANG_AN_MY))
					{
						LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 暗杠,牌:" << (int)cbActionCardData;
					}
					if((tip.bGangCount > 0) && (tip.wOperateCode & WIK_GANG_MY) && (tip.wOperateCode & WIK_GANG_MING_PENG_MY))
					{
						LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家:" << it->second.dwUserID << " 巴杠,牌:" << (int)cbActionCardData;
					}
					if(tip.wOperateCode & WIK_TING_MY)
					{
						//玩家能听牌记录玩家听牌的触发器
						m_eCanTingTrigger = Trigger_ZhuaCard;
						m_cbCanTingTrigger_ActionCardData = cbActionCardData;
						LOG_IF(WARNING, PRINT_LOG_INFO) << "提示玩家: " << it->second.dwUserID << " 听牌";
					}

					m_cbCurState = STATE_WAIT_SEND_OPERATE;
					m_dwCurOperatePlayer = it->first;
					SendTableData(it->first, SUB_S_OPERATE_TIP_SEND, &tip, sizeof(tip), true);

					if(it->second.bIsTing == CT_FALSE && it->second.cbPlayerAction & WIK_TING_MY)
					{
						TipPlayerMax(it->first);
					}

					if(bStartOperateTimeOutTimer)
					{
						m_pGameDeskPtr->SetGameTimer(IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD,
													 TIMER_OPERATE_TIME_OUT_AFTER_ZHUACARD * 1000,dwExpectPlayer);
					}
				}
			}
		}
	}
	else
	{
		LOG_IF(WARNING, PRINT_LOG_INFO) << "TipPlayerAction tipTrigger:" << tipTrigger;
	}

}

CT_BOOL CGameProcess::IsHaveAction_AfterZhuaCard(CT_DWORD dwZhuaCardPlayer, CT_BYTE cbZhuaCardData)
{
    //先清除所有玩家的动作
    for(MapChairInfo::iterator itClear = m_mapChairInfo.begin(); itClear != m_mapChairInfo.end(); itClear++)
    {
        itClear->second.clearPlayerAction();
    }

	MapChairInfo::iterator it = m_mapChairInfo.find(dwZhuaCardPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "IsHaveAction_AfterZhuaCard 通过椅子ID:" << dwZhuaCardPlayer << " 找不到玩家" << " 抓的牌是:"<<(int)cbZhuaCardData;
		return CT_FALSE;
	}

	CT_BOOL bHaveAction = CT_FALSE;
	//是否能听
	if(CanTing(dwZhuaCardPlayer))
	{
		m_eTingReason = TingTrigger::ZhuaCard;
		//如果能听，还要发送提示出牌消息，好让玩家在点击听牌后，能出牌。
		//TipPlayerOutCard(dwZhuaCardPlayer, cbZhuaCardData);
		//玩家取消听牌后超时不出牌，系统自动出的牌
		m_cbCancelTing_OutTime_OutCard = cbZhuaCardData;
	}

	//出牌后判断玩家是否能胡牌需要把玩家组合牌拷贝出来并且转换,因为玩家身上的weaveItem中cbWeaveKind成员  和  GameLogic中的行为值不一样
	CT_WORD wWeaveCount = 0;
	tagWeaveItem tempWeaveItem[MAX_WEAVE];
	for(int i = 0; i < MAX_WEAVE; i++)
	{
		tempWeaveItem[i].clearWeaveItem();
		if(it->second.weaveItem[i].cbWeaveKind == WIK_NULL_MY)
		{
			continue;
		}

		tempWeaveItem[i].cbCenterCard = it->second.weaveItem[i].cbCenterCard;
		tempWeaveItem[i].cbPublicCard = it->second.weaveItem[i].cbPublicCard;
		tempWeaveItem[i].wProvideUser = it->second.weaveItem[i].wProvideUser;
		memcpy(tempWeaveItem[i].cbCardData,it->second.weaveItem[i].cbCardData, sizeof(it->second.weaveItem[i].cbCardData));
		if(it->second.weaveItem[i].cbWeaveKind == WIK_RIGHT_MY)
		{
			tempWeaveItem[i].cbWeaveKind  = WIK_RIGHT;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_LEFT_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_LEFT;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_CENTER_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_CENTER;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_PENG;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_MY || it->second.weaveItem[i].cbWeaveKind == WIK_GANG_AN_MY
				|| it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_PENG_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_GANG;
		}
		else
		{
			LOG(ERROR) <<"组合牌转换的时候1, " <<it->second.weaveItem[i].cbWeaveKind;
		}

		wWeaveCount++;
	}

	//听牌之后不能杠的牌
	std::vector<CT_BYTE> vecCannotGangAfterTing;
	vecCannotGangAfterTing.clear();
	//是否能 暗杠 和 巴杠
	m_GameLogic.AnalyseGangCard(it->second.cbHandCardIndex, tempWeaveItem, wWeaveCount, it->second.canGangCard);
	for(int j = 0; j < MAX_WEAVE; j++)
	{
		if(it->second.canGangCard.dwGangCardType[j] & WIK_GANG_AN_MY)
		{
			if(it->second.bIsTing == CT_FALSE)
			{
				it->second.cbPlayerAction |= WIK_GANG_MY;
				it->second.cbPlayerAction |= WIK_GANG_AN_MY;
			}
			else
			{
				if(CanGangAfterTing(dwZhuaCardPlayer, it->second.canGangCard.cbCardData[j], WIK_GANG_AN_MY, 0))
				{
					it->second.cbPlayerAction |= WIK_GANG_MY;
					it->second.cbPlayerAction |= WIK_GANG_AN_MY;
				}
				else
				{
					//听牌之后不能杠的牌
					vecCannotGangAfterTing.push_back(it->second.canGangCard.cbCardData[j]);
				}
			}
		}
		else if( it->second.canGangCard.dwGangCardType[j] & WIK_GANG_MING_PENG_MY)
		{
			if(it->second.bIsTing == CT_FALSE)
			{
				it->second.cbPlayerAction |= WIK_GANG_MY;
				it->second.cbPlayerAction |= WIK_GANG_MING_PENG_MY;
			}
			else
			{
				if(CanGangAfterTing(dwZhuaCardPlayer, it->second.canGangCard.cbCardData[j], WIK_GANG_MING_PENG_MY, 0))
				{
					it->second.cbPlayerAction |= WIK_GANG_MY;
					it->second.cbPlayerAction |= WIK_GANG_MING_PENG_MY;
				}
				else
				{
					//听牌之后不能杠的牌
					vecCannotGangAfterTing.push_back(it->second.canGangCard.cbCardData[j]);
				}
			}
		}
	}

	if(vecCannotGangAfterTing.size() > 0)
	{
		//如果有听后不能杠的牌，那么就从能杠的牌中清除掉
		tagGangCardResult tempGangResult;
		memcpy(&tempGangResult, &it->second.canGangCard, sizeof(tagGangCardResult));

		for(size_t i = 0; i < MAX_WEAVE; i++)
		{
			if(tempGangResult.cbCardData[i] == INVALID_CARD_DATA)
			{
				continue;
			}

			for(size_t j = 0; j < vecCannotGangAfterTing.size(); j++)
			{
				if(tempGangResult.cbCardData[i] == vecCannotGangAfterTing[j])
				{
					tempGangResult.cbCardData[i] = INVALID_CARD_DATA;
					tempGangResult.cbCardCount--;
					tempGangResult.dwGangCardType[i] = 0;
					break;
				}
			}
		}

		it->second.canGangCard.clear();
		it->second.canGangCard.cbCardCount = tempGangResult.cbCardCount;
		int nCurIndex = 0;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			if(tempGangResult.cbCardData[i] != INVALID_CARD_DATA)
			{
				it->second.canGangCard.cbCardData[nCurIndex] = tempGangResult.cbCardData[i];
				it->second.canGangCard.dwGangCardType[nCurIndex] = tempGangResult.dwGangCardType[i];
				nCurIndex++;
			}
		}
	}

	//是否能吃胡
	CChiHuRight chr;
	//由于调用这个函数的时候，抓的牌已经在手牌中，所以先删除牌
	if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbZhuaCardData))
    {
	    LOG(ERROR) << "从玩家ID: "<< it->second.dwUserID << " 身上移除手牌: " << (int)cbZhuaCardData << " 失败";
    }
	CT_BYTE cbChiHu = m_GameLogic.AnalyseChiHuCard(it->second.cbHandCardIndex,tempWeaveItem, wWeaveCount, cbZhuaCardData, chr, it->second.m_AnalyseItemArray);
	//判断完胡牌后再加入手牌中
	if(CT_FALSE == m_GameLogic.AddCard(it->second.cbHandCardIndex, cbZhuaCardData))
	{
		LOG(ERROR) << "给玩家ID: "<< it->second.dwUserID << " 身上添加手牌: "<< (int)cbZhuaCardData << " 失败";
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//为判断胡牌的牌型番数 还有听牌做准备
	//获取能胡牌的具体数据
	//所有玩家手牌信息
	CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
	memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
	//CT_BYTE cbCurPlayerIndex = 0;
	//所有玩家弃牌信息
	CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
	memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
	CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
	memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
	//所有玩家的组合项
	tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
	for(int m = 0 ; m < GAME_PLAYER; m++)
	{
		for(int n = 0; n < MAX_WEAVE; n++)
		{
			cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
		}
	}
	CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
	memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

	//所有玩家的听牌状态
	CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
	memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

	//所有玩家是否天听
	CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
	memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

	for(MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
	{
		//获取所有玩家手牌信息
		for(int k = 0 ; k < MAX_INDEX; k++)
		{
			cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
		}

		//获取所有玩家弃牌信息
		for(int k = 0; k < itChair->second.cbDiscardCount; k++)
		{
			cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
		}
		cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

		//获取所有玩家听牌状态
        cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

        //获取所有玩家的天听状态
		bAllPlayerIsTianTing[itChair->first] = itChair->second.bTianTing;

		//获取所有玩家的组合项
		CT_BYTE cbWeaveItemCount = 0;
		for(int k = 0; k < MAX_WEAVE; k++)
		{
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}
			memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData, sizeof(itChair->second.weaveItem[k].cbCardData));
			cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
			cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
			if(itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
			}
			else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
					itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY)
			{
				cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
			}
			cbWeaveItemCount++;
		}
		cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
	}
	CT_CHAR szGameRoomName[LEN_SERVER] = "test";
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//是否能胡
	if(cbChiHu == WIK_CHI_HU)
	{
		it->second.cbPlayerAction |= WIK_HU_MY;
		it->second.cbPlayerAction |= WIK_HU_ZI_MO_MY;

		//能胡并且听牌：加倍
		if(it->second.bIsTing)
		{
			it->second.cbPlayerAction |= WIK_JIABEI;
		}

		//获取能胡的牌型 和 番数
		tagAnalyseItem tempItem;
		//CT_BOOL bZiMo = CT_TRUE;
		it->second.bIsSuanZiMo = CT_TRUE;
		it->second.llHuCardType = m_GameLogic.GetHuType(it->first, m_dwBanker, INVALID_CHAIR, it->first, cbZhuaCardData, cbAllPlayerHandCardIndex,
		        cbAllPlayerDiscardCard, cbAllPlayerDiscardCount, m_cbAllPlayerOutCardCount, cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount,
		        cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0, GetMenFeng(it->first), m_cbLeftCardCount, m_bGangStatus, szGameRoomName,
		        it->second.m_AnalyseItemArray, it->second.wHuCardFan, tempItem, it->second.bIsSuanZiMo, CT_FALSE);
        LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家ID:" << it->second.dwUserID << " 能胡的牌型是:"<<it->second.llHuCardType;
	}

	if(it->second.cbPlayerAction != WIK_NULL_MY)
	{
		//玩家实施行为的牌数据
		it->second.cbPlayerActionCardData = cbZhuaCardData;
		//玩家实施行为的时候，这个行为的引发者
		it->second.dwActionProvider = dwZhuaCardPlayer;
		bHaveAction = CT_TRUE;
	}

	return bHaveAction;
}

CT_BOOL CGameProcess::IsHaveQiangGangHu_AfterBaGang(CT_DWORD dwBaGangPlayer, CT_BYTE cbBaGangCardData)
{
    CT_BOOL bHaveAction = CT_FALSE;

	MapChairInfo::iterator itBaGangPlayer = m_mapChairInfo.find(dwBaGangPlayer);
	if(itBaGangPlayer == m_mapChairInfo.end())
	{
		LOG(ERROR) << "通过椅子ID: " << dwBaGangPlayer << " 找不到玩家";
		return CT_FALSE;
	}

/*
 * CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbBaGangCardData);
	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(itBaGangPlayer->second.cbHandCardIndex, cbCardIndex);
	if(1 !=cbCardCount)
	{
		LOG(ERROR) << "玩家:" << itBaGangPlayer->second.dwUserID << " 巴杠牌: "<< (int)cbBaGangCardData << " 但是有:" << (int)cbCardCount << " 张";
		return CT_FALSE;
	}

	if(!(itBaGangPlayer->second.cbPlayerAction & WIK_GANG_MING_PENG_MY))
	{
		LOG(ERROR) << "玩家: "<<itBaGangPlayer->second.dwUserID << " 不能巴杠";
		return CT_FALSE;
	}*/

	//除了巴杠玩家,清除其他玩家的行为
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		if(it->first == dwBaGangPlayer)
		{
			continue;
		}

		it->second.clearPlayerAction();
	}

	//判断除巴杠者之外其他玩家能够胡牌
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		if(it->first == dwBaGangPlayer)
		{
			continue;
		}

		//出牌后判断玩家是否能胡牌需要把玩家组合牌拷贝出来并且转换,因为玩家身上的weaveItem中cbWeaveKind成员  和  GameLogic中的行为值不一样
		tagWeaveItem tempWeaveItem[MAX_WEAVE];
		CT_BYTE cbWeaveItemCount = 0;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			tempWeaveItem[i].clearWeaveItem();
			if(it->second.weaveItem[i].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}
			tempWeaveItem[i].cbCenterCard = it->second.weaveItem[i].cbCenterCard;
			tempWeaveItem[i].cbPublicCard = it->second.weaveItem[i].cbPublicCard;
			tempWeaveItem[i].wProvideUser = it->second.weaveItem[i].wProvideUser;
			memcpy(tempWeaveItem[i].cbCardData,it->second.weaveItem[i].cbCardData, sizeof(it->second.weaveItem[i].cbCardData));
			if(it->second.weaveItem[i].cbWeaveKind == WIK_RIGHT_MY)
			{
				tempWeaveItem[i].cbWeaveKind  = WIK_RIGHT;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_LEFT_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_LEFT;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_CENTER_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_CENTER;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_PENG;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_MY || it->second.weaveItem[i].cbWeaveKind == WIK_GANG_AN_MY
					|| it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_PENG_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_GANG;
			}
			else
			{
				LOG(ERROR) <<"组合牌转换的时候, " <<it->second.weaveItem[i].cbWeaveKind;
			}

			cbWeaveItemCount++;
		}

		//是否能胡
		CChiHuRight chr;
		CT_BYTE cbHu = m_GameLogic.AnalyseChiHuCard(it->second.cbHandCardIndex, tempWeaveItem,cbWeaveItemCount, cbBaGangCardData, chr, it->second.m_AnalyseItemArray);
		if(cbHu == WIK_CHI_HU)
		{
			it->second.cbPlayerAction |= WIK_HU_MY;
			it->second.cbPlayerAction |= WIK_HU_FANG_PAO_MY;
			it->second.bCanQiangGangHu = CT_TRUE;

			if(it->second.bIsTing)
			{
				//能胡并且已经听牌：加倍
				it->second.cbPlayerAction |= WIK_JIABEI;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(CT_FALSE == m_GameLogic.AddCard(it->second.cbHandCardIndex, cbBaGangCardData))
			{
				LOG(ERROR) << "给玩家ID: "<< it->second.dwUserID << " 身上添加牌1: "<< (int)cbBaGangCardData << " 失败";
			}
			//获取能胡牌的具体数据
			//所有玩家手牌信息
			CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
			memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
			//CT_BYTE cbCurPlayerIndex = 0;
			//所有玩家弃牌信息
			CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
			memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
			CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
			memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
			//所有玩家的组合项
			tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
			for(int m = 0 ; m < GAME_PLAYER; m++)
			{
				for(int n = 0; n < MAX_WEAVE; n++)
				{
					cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
				}
			}
			CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
			memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

			//所有玩家的听牌状态
			CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
			memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

			//所有玩家是否天听
			CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
			memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

			for(MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
			{
				//获取所有玩家手牌信息
				for(int k = 0 ; k < MAX_INDEX; k++)
				{
					cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
				}

				//获取所有玩家弃牌信息
				for(int k = 0; k < itChair->second.cbDiscardCount; k++)
				{
					cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
				}
				cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

				//获取所有玩家的听牌状态
				cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

				//获取所有玩家的天庭状态
				bAllPlayerIsTianTing[itChair->first] = itChair->second.bTianTing;

				//获取所有玩家的组合项
				CT_BYTE cbWeaveItemCount = 0;
				for(int k = 0; k < MAX_WEAVE; k++)
				{
					if(itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY)
					{
						continue;
					}
					memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData, sizeof(itChair->second.weaveItem[k].cbCardData));
					cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
					cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
					cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
					if(itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
							itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
							itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
					}
					cbWeaveItemCount++;
				}
				cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
			}

			CT_CHAR szGameRoomName[LEN_SERVER] = "test";
			tagAnalyseItem tempItem;
			it->second.bIsSuanZiMo = CT_FALSE;
			it->second.llHuCardType = m_GameLogic.GetHuType(it->first, m_dwBanker, dwBaGangPlayer, dwBaGangPlayer,
			         cbBaGangCardData, cbAllPlayerHandCardIndex, cbAllPlayerDiscardCard, cbAllPlayerDiscardCount,
					 m_cbAllPlayerOutCardCount, cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount,
					 cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0, GetMenFeng(it->first),
					 m_cbLeftCardCount, m_bGangStatus, szGameRoomName, it->second.m_AnalyseItemArray,
					 it->second.wHuCardFan, tempItem, it->second.bIsSuanZiMo, CT_FALSE);

			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家ID:" << it->second.dwUserID << " 能胡的牌型是1:"<<it->second.llHuCardType;
			if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbBaGangCardData))
			{
				LOG(ERROR) << "从玩家ID: "<< it->second.dwUserID << " 身上删除牌1: "<< (int)cbBaGangCardData << " 失败";
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}

		if(it->second.cbPlayerAction != WIK_NULL_MY)
		{
			//玩家实施行为的牌数据
			it->second.cbPlayerActionCardData = cbBaGangCardData;
			//玩家实施行为的时候，这个行为的引发者
			it->second.dwActionProvider = dwBaGangPlayer;

            bHaveAction = CT_TRUE;
		}
	}

	return bHaveAction;
}

CT_BOOL CGameProcess::IsHaveAction_AfterOutCard(CT_DWORD dwOutCardPlayer, CT_BYTE cbOutCardData)
{
	CT_BOOL bHaveAction = CT_FALSE;

    for(MapChairInfo::iterator itClear = m_mapChairInfo.begin(); itClear != m_mapChairInfo.end(); itClear++)
    {
        //先清除所有玩家的动作
        itClear->second.clearPlayerAction();
    }

    //判断除了出牌者以外其他人是否能有行为。
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
    {
		//出牌者不判断
	    if(it->first == dwOutCardPlayer)
        {
            continue;
        }

        //是否能碰
        if(it->second.bIsTing == CT_FALSE)
        {
			CT_BYTE cbPeng = m_GameLogic.EstimatePengCard(it->second.cbHandCardIndex, cbOutCardData);
			if (cbPeng == WIK_PENG) {
				it->second.cbPlayerAction |= WIK_PENG_MY;
			}
		}

        //是否能杠
        CT_BYTE cbGang = m_GameLogic.EstimateGangCard(it->second.cbHandCardIndex, cbOutCardData);
        if (cbGang == WIK_GANG)
        {
            if(it->second.bIsTing == CT_FALSE)
            {
                it->second.cbPlayerAction |= WIK_GANG_MY;
                it->second.cbPlayerAction |= WIK_GANG_MING_MY;
            }
            else
            {
                if(CanGangAfterTing(dwOutCardPlayer, cbOutCardData, WIK_GANG_MING_MY, it->first))
                {
                    it->second.cbPlayerAction |= WIK_GANG_MY;
                    it->second.cbPlayerAction |= WIK_GANG_MING_MY;
                }
            }
        }

		if(it->second.bIsTing == CT_FALSE)
		{
			if(m_GameLogic.GetCardColor(cbOutCardData) == Color_WAN)
			{
				//是否能吃
				CT_BYTE cbChi = m_GameLogic.EstimateEatCard(it->second.cbHandCardIndex, cbOutCardData);
				if(cbChi & WIK_LEFT || cbChi & WIK_RIGHT || cbChi & WIK_CENTER)
				{
					it->second.cbPlayerAction |= WIK_CHI_MY;
					if(cbChi & WIK_LEFT)
						it->second.cbPlayerAction |= WIK_LEFT_MY;
					if(cbChi & WIK_CENTER)
						it->second.cbPlayerAction |= WIK_CENTER_MY;
					if(cbChi & WIK_RIGHT)
						it->second.cbPlayerAction |= WIK_RIGHT_MY;
				}
			}
			else
			{
				if(cbOutCardData >= 0x01 && cbOutCardData <= 0x09)
				{
					LOG(ERROR) << "牌:"<<(int)cbOutCardData << " 被判断为不是万字";
				}
			}
		}

		//出牌后判断玩家是否能胡牌需要把玩家组合牌拷贝出来并且转换,因为玩家身上的weaveItem中cbWeaveKind成员  和  GameLogic中的行为值不一样
		tagWeaveItem tempWeaveItem[MAX_WEAVE];
		CT_BYTE cbWeaveItemCount = 0;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			tempWeaveItem[i].clearWeaveItem();
			if(it->second.weaveItem[i].cbWeaveKind == WIK_NULL_MY)
			{
				continue;
			}
			tempWeaveItem[i].cbCenterCard = it->second.weaveItem[i].cbCenterCard;
			tempWeaveItem[i].cbPublicCard = it->second.weaveItem[i].cbPublicCard;
			tempWeaveItem[i].wProvideUser = it->second.weaveItem[i].wProvideUser;
			memcpy(tempWeaveItem[i].cbCardData,it->second.weaveItem[i].cbCardData, sizeof(it->second.weaveItem[i].cbCardData));
			if(it->second.weaveItem[i].cbWeaveKind == WIK_RIGHT_MY)
			{
				tempWeaveItem[i].cbWeaveKind  = WIK_RIGHT;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_LEFT_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_LEFT;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_CENTER_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_CENTER;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_PENG;
			}
			else if(it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_MY || it->second.weaveItem[i].cbWeaveKind == WIK_GANG_AN_MY
				|| it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_PENG_MY)
			{
				tempWeaveItem[i].cbWeaveKind = WIK_GANG;
			}
			else
			{
				LOG(ERROR) <<"组合牌转换的时候, " <<it->second.weaveItem[i].cbWeaveKind;
			}

			cbWeaveItemCount++;
		}

		//是否能胡
		CChiHuRight chr;
		CT_BYTE cbHu = m_GameLogic.AnalyseChiHuCard(it->second.cbHandCardIndex, tempWeaveItem,cbWeaveItemCount, cbOutCardData, chr, it->second.m_AnalyseItemArray);
		if(cbHu == WIK_CHI_HU)
		{
			it->second.cbPlayerAction |= WIK_HU_MY;
			it->second.cbPlayerAction |= WIK_HU_FANG_PAO_MY;

			if(it->second.bIsTing)
			{
				//能胡并且已经听牌：加倍
				it->second.cbPlayerAction |= WIK_JIABEI;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(CT_FALSE == m_GameLogic.AddCard(it->second.cbHandCardIndex, cbOutCardData))
			{
				LOG(ERROR) << "给玩家ID: "<< it->second.dwUserID << " 身上添加牌: "<< (int)cbOutCardData << " 失败";
			}
			//别人出的牌加入到自己手中，要从别人的弃牌中删除
			MapChairInfo::iterator itDelDisCard = m_mapChairInfo.find(dwOutCardPlayer);
			if(itDelDisCard != m_mapChairInfo.end())
			{
                itDelDisCard->second.cbDiscardCard[itDelDisCard->second.cbDiscardCount-1] = INVALID_CARD_DATA;
                itDelDisCard->second.cbDiscardCount--;

				//这里要改
				/*int loop = itDelDisCard->second.cbDiscardCount > MAX_OUT_NUM ? MAX_OUT_NUM : itDelDisCard->second.cbDiscardCount;
				for(int k = loop-1; k >= 0; k--)
				{
					if(itDelDisCard->second.cbDiscardCard[k] == cbOutCardData)
					{
						itDelDisCard->second.cbDiscardCard[k] = INVALID_CARD_DATA;
						itDelDisCard->second.cbDiscardCount--;
					}
				}*/
			}

			//获取能胡牌的具体数据
			//所有玩家手牌信息
			CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
			memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
			//CT_BYTE cbCurPlayerIndex = 0;
			//所有玩家弃牌信息
			CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
			memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
			CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
			memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
			//所有玩家的组合项
			tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
			for(int m = 0 ; m < GAME_PLAYER; m++)
			{
				for(int n = 0; n < MAX_WEAVE; n++)
				{
					cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
				}
			}
			CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
			memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

			//所有玩家的听牌状态
			CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
			memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

			//所有玩家是否天听
			CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
			memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

			for(MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
			{
				//获取所有玩家手牌信息
				for(int k = 0 ; k < MAX_INDEX; k++)
				{
					cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
				}

				//获取所有玩家弃牌信息
				for(int k = 0; k < itChair->second.cbDiscardCount; k++)
				{
					cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
				}
				cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

				//获取所有玩家的听牌状态
                cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

                //获取所有玩家的天庭状态
				bAllPlayerIsTianTing[itChair->first] = itChair->second.bTianTing;

				//获取所有玩家的组合项
				CT_BYTE cbWeaveItemCount = 0;
				for(int k = 0; k < MAX_WEAVE; k++)
				{
					if(itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY)
					{
						continue;
					}
					memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData, sizeof(itChair->second.weaveItem[k].cbCardData));
					cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
					cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
					cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
					if(itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
					}
					else if(itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
							itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
							itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY)
					{
						cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
					}
					cbWeaveItemCount++;
				}
				cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
			}

			CT_CHAR szGameRoomName[LEN_SERVER] = "test";
			tagAnalyseItem tempItem;

			it->second.bIsSuanZiMo = CT_FALSE;
			it->second.llHuCardType = m_GameLogic.GetHuType
			        (it->first, m_dwBanker, dwOutCardPlayer, dwOutCardPlayer, cbOutCardData, cbAllPlayerHandCardIndex, cbAllPlayerDiscardCard, cbAllPlayerDiscardCount,
					m_cbAllPlayerOutCardCount, cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount, cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0, GetMenFeng(it->first),
					m_cbLeftCardCount, m_bGangStatus, szGameRoomName, it->second.m_AnalyseItemArray, it->second.wHuCardFan, tempItem, it->second.bIsSuanZiMo, CT_FALSE);
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家ID:" << it->second.dwUserID << " 能胡的牌型是:"<<it->second.llHuCardType;
			if(CT_FALSE == m_GameLogic.RemoveCard(it->second.cbHandCardIndex, cbOutCardData))
			{
				LOG(ERROR) << "从玩家ID: "<< it->second.dwUserID << " 身上删除牌: "<< (int)cbOutCardData << " 失败";
			}
			//放回出牌者的弃牌数据中
			MapChairInfo::iterator itAddDisCard = m_mapChairInfo.find(dwOutCardPlayer);
			if(itAddDisCard != m_mapChairInfo.end())
			{
                itAddDisCard->second.cbDiscardCard[itAddDisCard->second.cbDiscardCount] = cbOutCardData;
                itAddDisCard->second.cbDiscardCount++;

				//这里要改
				/*if(itAddDisCard->second.cbDiscardCount < MAX_OUT_NUM)
				{
					itAddDisCard->second.cbDiscardCard[itAddDisCard->second.cbDiscardCount] = cbOutCardData;
					itAddDisCard->second.cbDiscardCount++;
				}
				else
				{
					LOG(ERROR) << "严重错误,玩家:"<<itAddDisCard->second.dwUserID << " 出牌区有: " << itAddDisCard->second.cbDiscardCount << " 张";
				}*/
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}

        if(it->second.cbPlayerAction != WIK_NULL_MY)
		{
        	//玩家实施行为的牌数据
        	it->second.cbPlayerActionCardData = cbOutCardData;
        	//玩家实施行为的时候，这个行为的引发者
        	it->second.dwActionProvider = dwOutCardPlayer;
			bHaveAction = CT_TRUE;
		}
    }

    return bHaveAction;
}

CT_VOID CGameProcess::PlayerZhuaCard(Action_Zhua action, CT_BYTE cbPlayer)
{
	if(m_cbLeftCardCount == 0)
	{
		LOG_IF(WARNING, PRINT_LOG_INFO) << "牌摸完结算";
		//没牌了结束本局
		OnEventGameEnd(INVALID_CHAIR,GER_NORMAL);
		return;
	}

	CT_WORD wCurrentZhuaCardPlayer = INVALID_CHAIR_ID;
	if(action == Action_OutCard)
	{
		//玩家出牌引发了玩家抓牌,确定下一家抓牌者
		//如果是出牌引发的抓牌,那么抓牌玩家就是出牌玩家的下一家
		wCurrentZhuaCardPlayer = (cbPlayer + 1) % GAME_PLAYER;
	}
	else if(action == Action_PassCard)
	{
		//玩家过牌引发了抓牌，抓牌者就是过牌者
		wCurrentZhuaCardPlayer = cbPlayer;
	}
	else if(action == Action_GangCard)
	{
		wCurrentZhuaCardPlayer = cbPlayer;
	}
	else if(action == Action_TingCard)
	{
		wCurrentZhuaCardPlayer = (cbPlayer + 1) % GAME_PLAYER;
	}
	else if(action == Action_JiaBei)
	{
		wCurrentZhuaCardPlayer = cbPlayer;
	}
	else
	{
		LOG(ERROR) << "严重BUG,抓牌时行为错误: "<< (int)Action_TingCard;
		wCurrentZhuaCardPlayer = (cbPlayer + 1) % GAME_PLAYER;
	}

	MapChairInfo::iterator itZhuaCardPlayer = m_mapChairInfo.find(wCurrentZhuaCardPlayer);
	if(itZhuaCardPlayer == m_mapChairInfo.end())
	{
		LOG(ERROR) << "玩家抓牌的时候，通过椅子ID:" << wCurrentZhuaCardPlayer << " 找不到玩家了";
		return;
	}

	CMD_S_SendCard zhuaCard;
	zhuaCard.wSendCardUser = wCurrentZhuaCardPlayer;
	zhuaCard.cbCardId = PlayerFetchOneCard(wCurrentZhuaCardPlayer);
	zhuaCard.wRemainNum = m_cbLeftCardCount;
	m_cbZhuaCardData = zhuaCard.cbCardId;
	m_dwCurZhuaCardPlayer = wCurrentZhuaCardPlayer;
    m_cbLastZhuaCardData = zhuaCard.cbCardId;

	CMD_S_SendCard zhuaCardOther;
	zhuaCardOther.wSendCardUser = wCurrentZhuaCardPlayer;
	zhuaCardOther.cbCardId = 0;
	zhuaCardOther.wRemainNum = m_cbLeftCardCount;

	m_cbAllPlayerZhuaCardCount++;
	//把抓的牌发下去
	for(MapChairInfo::iterator it = m_mapChairInfo.begin(); it != m_mapChairInfo.end(); it++)
	{
		if(it->first == wCurrentZhuaCardPlayer)
		{
			SendTableData(it->first, SUB_S_SEND_CARD, &zhuaCard, sizeof(zhuaCard), true);
			LOG_IF(WARNING, PRINT_LOG_INFO) << "玩家:"<<it->second.dwUserID << " 抓牌:" << (int)zhuaCard.cbCardId;
		}
		else
		{
			if(itZhuaCardPlayer->second.bIsTing && m_dwTingCardPlayerCnt >= GAME_PLAYER)
			{
				//抓牌者听牌了，那么就把抓的牌也发给其他人
				SendTableData(it->first, SUB_S_SEND_CARD, &zhuaCard, sizeof(zhuaCard), false);
			}
			else
			{
				SendTableData(it->first, SUB_S_SEND_CARD, &zhuaCardOther, sizeof(zhuaCardOther), false);
			}
		}
	}

	//当前操作玩家是抓牌玩家
	m_dwCurOperatePlayer = wCurrentZhuaCardPlayer;

	ClearMaxActionData();

	//判断抓牌的玩家是否有行为
	if(IsHaveAction_AfterZhuaCard(wCurrentZhuaCardPlayer, m_cbZhuaCardData))
	{
		//只要抓牌产生了行为就提示玩家出牌
		TipPlayerOutCard(wCurrentZhuaCardPlayer, m_cbZhuaCardData, CT_FALSE);
		m_startOperateTime = Utility::GetTick();
		m_cbCurState = STATE_WAIT_SEND_OPERATE;
		//抓牌后提示玩家行为
		TipPlayerAction(wCurrentZhuaCardPlayer, m_cbZhuaCardData, Trigger_ZhuaCard);
	}
	else
	{
	    MapChairInfo::iterator itZhuaCard = m_mapChairInfo.find(wCurrentZhuaCardPlayer);
	    if(itZhuaCard != m_mapChairInfo.end())
        {
	        if(itZhuaCard->second.bIsTing && itZhuaCard->second.bIsOutFirstCard_AfterTing == CT_FALSE)
            {
	            //听牌后自动打牌
	            m_dwAutoOutCardPlayer = wCurrentZhuaCardPlayer;
	            m_dWAutoOutCardData = m_cbZhuaCardData;
                m_dwCurOperatePlayer = wCurrentZhuaCardPlayer;
                m_cbCurState = STATE_WAIT_OUT;
	            m_pGameDeskPtr->SetGameTimer(IDI_AUTO_OUT_CARD_AFTER_TING, TIMER_AUTO_OUT_CARD_AFTER_TING);
            }
            else
            {
				//提示出牌
				TipPlayerOutCard(wCurrentZhuaCardPlayer,m_cbZhuaCardData, CT_TRUE);
            }
        }
	}

	if(wCurrentZhuaCardPlayer != m_dwBanker && m_bXianPlayerFirstZhuaCard)
    {
        MapChairInfo::iterator itXianPlayer = m_mapChairInfo.find(wCurrentZhuaCardPlayer);
        if(itXianPlayer != m_mapChairInfo.end())
        {
            if(itXianPlayer->second.cbPlayerAction & WIK_TING_MY)
            {
				m_bCanTianTingXian = CT_TRUE;
            }
        }

	    //不是庄家便是闲家
	    m_bXianPlayerFirstZhuaCard = CT_FALSE;
    }
}

CT_BOOL CGameProcess::CanGangAfterTing(CT_DWORD dwChairID, CT_BYTE cbCurCardData, CT_DWORD dwGangType, CT_DWORD dwMingGangUser)
{
	if(dwGangType != WIK_GANG_AN_MY && dwGangType != WIK_GANG_MING_MY && dwGangType != WIK_GANG_MING_PENG_MY)
	{
		return CT_FALSE;
	}

	MapChairInfo::iterator it;
	if(dwGangType == WIK_GANG_AN_MY)
	{
		it = m_mapChairInfo.find(dwChairID);
	}
	else if(dwGangType == WIK_GANG_MING_MY)
	{
		it = m_mapChairInfo.find(dwMingGangUser);
	}
	else if(dwGangType == WIK_GANG_MING_PENG_MY)
    {
        it = m_mapChairInfo.find(dwChairID);
    }
	if(it == m_mapChairInfo.end())
	{
		return CT_FALSE;
	}

	//拷贝手牌
	CT_BYTE cbTempHandCardIndex[MAX_INDEX];
	memset(cbTempHandCardIndex, 0, sizeof(cbTempHandCardIndex));
	memcpy(cbTempHandCardIndex, it->second.cbHandCardIndex, sizeof(CT_BYTE) * MAX_INDEX);

	//拷贝组合牌
	CT_WORD wWeaveCount = 0;
	tagWeaveItem tempWeaveItem[MAX_WEAVE];
	for(int i = 0; i < MAX_WEAVE; i++)
	{
		tempWeaveItem[i].clearWeaveItem1();
		if(it->second.weaveItem[i].cbWeaveKind == WIK_NULL_MY)
		{
			continue;
		}

		tempWeaveItem[i].cbCenterCard = it->second.weaveItem[i].cbCenterCard;
		tempWeaveItem[i].cbPublicCard = it->second.weaveItem[i].cbPublicCard;
		tempWeaveItem[i].wProvideUser = it->second.weaveItem[i].wProvideUser;
		memcpy(tempWeaveItem[i].cbCardData,it->second.weaveItem[i].cbCardData, sizeof(it->second.weaveItem[i].cbCardData));
		if(it->second.weaveItem[i].cbWeaveKind == WIK_RIGHT_MY)
		{
			tempWeaveItem[i].cbWeaveKind  = WIK_RIGHT;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_LEFT_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_LEFT;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_CENTER_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_CENTER;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_PENG_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_PENG;
		}
		else if(it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_MY || it->second.weaveItem[i].cbWeaveKind == WIK_GANG_AN_MY
				|| it->second.weaveItem[i].cbWeaveKind == WIK_GANG_MING_PENG_MY)
		{
			tempWeaveItem[i].cbWeaveKind = WIK_GANG;
		}
		else
		{
			LOG(ERROR) <<"组合牌转换的时候11111, " <<it->second.weaveItem[i].cbWeaveKind;
		}

		wWeaveCount++;
	}

	if(dwGangType == WIK_GANG_AN_MY)
	{
		CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbCurCardData);
		if(cbTempHandCardIndex[cbCardIndex] == 4)
		{
			cbTempHandCardIndex[cbCardIndex] = 0;
		}
		else
		{
			return CT_FALSE;
		}

		if(wWeaveCount < MAX_WEAVE)
		{
			tempWeaveItem[wWeaveCount].cbCenterCard = cbCurCardData;
			tempWeaveItem[wWeaveCount].cbWeaveKind = WIK_GANG;
			tempWeaveItem[wWeaveCount].cbPublicCard = CT_FALSE;
			tempWeaveItem[wWeaveCount].wProvideUser = dwChairID;
			tempWeaveItem[wWeaveCount].cbCardData[0] = cbCurCardData;
			tempWeaveItem[wWeaveCount].cbCardData[1] = cbCurCardData;
			tempWeaveItem[wWeaveCount].cbCardData[2] = cbCurCardData;
			tempWeaveItem[wWeaveCount].cbCardData[3] = cbCurCardData;

			wWeaveCount++;
		}
		else
		{
			return CT_FALSE;
		}
	}
	else if(dwGangType == WIK_GANG_MING_MY)
    {
	    CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbCurCardData);
	    if(cbTempHandCardIndex[cbCardIndex] == 3)
        {
            cbTempHandCardIndex[cbCardIndex] = 0;
        }
        else
        {
            return CT_FALSE;
        }

        if(wWeaveCount < MAX_WEAVE)
        {
            tempWeaveItem[wWeaveCount].cbCenterCard = cbCurCardData;
            tempWeaveItem[wWeaveCount].cbWeaveKind = WIK_GANG;
            tempWeaveItem[wWeaveCount].cbPublicCard = CT_TRUE;
            tempWeaveItem[wWeaveCount].wProvideUser = dwChairID;
            tempWeaveItem[wWeaveCount].cbCardData[0] = cbCurCardData;
            tempWeaveItem[wWeaveCount].cbCardData[1] = cbCurCardData;
            tempWeaveItem[wWeaveCount].cbCardData[2] = cbCurCardData;
            tempWeaveItem[wWeaveCount].cbCardData[3] = cbCurCardData;

            wWeaveCount++;
        }
        else
        {
            return CT_FALSE;
        }
    }
    else if(dwGangType == WIK_GANG_MING_PENG_MY)
    {
        CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbCurCardData);
        if(cbTempHandCardIndex[cbCardIndex] == 1)
        {
            cbTempHandCardIndex[cbCardIndex] = 0;
        }
        else
        {
           return CT_FALSE;
        }

        CT_BOOL bFindPeng = CT_FALSE;
        for(int i = 0; i < MAX_WEAVE; i++)
        {
            if(tempWeaveItem[i].cbWeaveKind == WIK_PENG && tempWeaveItem[i].cbCenterCard == cbCurCardData)
            {
                tempWeaveItem[i].cbWeaveKind = WIK_GANG;
                tempWeaveItem[i].cbCenterCard = cbCurCardData;
                tempWeaveItem[i].cbPublicCard = CT_TRUE;
                tempWeaveItem[i].wProvideUser = dwChairID;
                tempWeaveItem[i].cbCardData[0] = cbCurCardData;
                tempWeaveItem[i].cbCardData[1] = cbCurCardData;
                tempWeaveItem[i].cbCardData[2] = cbCurCardData;
                tempWeaveItem[i].cbCardData[3] = cbCurCardData;
                bFindPeng = CT_TRUE;
                break;
            }
        }

        if(bFindPeng == CT_FALSE)
        {
            return CT_FALSE;
        }
    }

	CT_BYTE cbCanHu[MAX_HU_CARD_COUNT];
	memset(cbCanHu, INVALID_CARD_DATA, sizeof(cbCanHu));
	CT_INT32 nCurIndex = 0;
	for(int i = 0; i < MAX_INDEX - MAX_HUA_CARD; i++)
	{
		if(WIK_LISTEN == m_GameLogic.AnalyseTingCard13Ex(cbTempHandCardIndex, tempWeaveItem, wWeaveCount, i))
		{
			cbCanHu[nCurIndex] = m_GameLogic.SwitchToCardData(i);
			nCurIndex++;
			if(nCurIndex >= MAX_HU_CARD_COUNT)
			{
				break;
			}
		}
	}

	int nTempTingHu = 0;
	for(int i = 0; i < MAX_HU_CARD_COUNT; i++)
	{
		if(it->second.cbCanHuCard_AfterTing[i] != INVALID_CARD_DATA)
		{
			nTempTingHu++;
		}
	}
	if(nTempTingHu != nCurIndex)
	{
		return CT_FALSE;
	}
	if(nTempTingHu == 0 || nCurIndex == 0)
	{
		return CT_FALSE;
	}

	for(int i = 0; i < MAX_HU_CARD_COUNT; i++)
	{
		if(it->second.cbCanHuCard_AfterTing[i] == INVALID_CARD_DATA)
		{
			continue;
		}

		CT_BOOL bFind = CT_FALSE;
		for(int j = 0; j < MAX_HU_CARD_COUNT; j++)
		{
			if(cbCanHu[j] == INVALID_CARD_DATA)
			{
				continue;
			}

			if(it->second.cbCanHuCard_AfterTing[i] == cbCanHu[j])
			{
				bFind = CT_TRUE;
				break;
			}
		}

		if(bFind == CT_FALSE)
		{
			return CT_FALSE;
		}
	}

	return CT_TRUE;
}

CT_VOID CGameProcess::ClearMaxActionData()
{
	m_dwMaxAction = WIK_NULL_MY;
	m_dwMaxActionPlayer = INVALID_CHAIR_ID;
}

CT_BOOL CGameProcess::CanTing(CT_DWORD dwChairID)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(dwChairID);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "CanTing，通过椅子ID ：" << dwChairID << " 找不到玩家";
		return CT_FALSE;
	}

	//打牌之前判断是否能听
	if(it->second.bIsTing == CT_FALSE)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		CT_BYTE cbAllPlayerHandCardIndex[GAME_PLAYER][MAX_INDEX];
		memset(cbAllPlayerHandCardIndex, 0, sizeof(cbAllPlayerHandCardIndex));
		//CT_BYTE cbCurPlayerIndex = 0;
		//所有玩家弃牌信息
		CT_BYTE cbAllPlayerDiscardCard[GAME_PLAYER][60];
		memset(cbAllPlayerDiscardCard, INVALID_CARD_DATA, sizeof(cbAllPlayerDiscardCard));
		CT_BYTE cbAllPlayerDiscardCount[GAME_PLAYER];
		memset(cbAllPlayerDiscardCount, 0, sizeof(cbAllPlayerDiscardCount));
		//所有玩家的组合项
		tagWeaveItem cbAllPlayerWeaveItemArray[GAME_PLAYER][MAX_WEAVE];
		for (int m = 0; m < GAME_PLAYER; m++) {
			for (int n = 0; n < MAX_WEAVE; n++) {
				cbAllPlayerWeaveItemArray[m][n].clearWeaveItem1();
			}
		}
		CT_BYTE cbAllPlayerWeaveItemCount[GAME_PLAYER];
		memset(cbAllPlayerWeaveItemCount, 0, sizeof(cbAllPlayerWeaveItemCount));

		//所有玩家的听牌状态
		CT_BYTE cbAllPlayerListenStatus[GAME_PLAYER];
		memset(cbAllPlayerListenStatus, 0, sizeof(cbAllPlayerListenStatus));

		//所有玩家是否天听
		CT_BOOL bAllPlayerIsTianTing[GAME_PLAYER];
		memset(bAllPlayerIsTianTing, 0, sizeof(bAllPlayerIsTianTing));

		for (MapChairInfo::iterator itChair = m_mapChairInfo.begin(); itChair != m_mapChairInfo.end(); itChair++)
		{
			//获取所有玩家手牌信息
			for (int k = 0; k < MAX_INDEX; k++)
			{
				cbAllPlayerHandCardIndex[itChair->first][k] = itChair->second.cbHandCardIndex[k];
			}

			//获取所有玩家弃牌信息
			for (int k = 0; k < itChair->second.cbDiscardCount; k++)
			{
				cbAllPlayerDiscardCard[itChair->first][k] = itChair->second.cbDiscardCard[k];
			}
			cbAllPlayerDiscardCount[itChair->first] = itChair->second.cbDiscardCount;

			//获取所有玩家听牌状态
			cbAllPlayerListenStatus[itChair->first] = itChair->second.bIsTing;

			//获取所有玩家的组合项
			CT_BYTE cbWeaveItemCount = 0;
			for (int k = 0; k < MAX_WEAVE; k++) {
				if (itChair->second.weaveItem[k].cbWeaveKind == WIK_NULL_MY) {
					continue;
				}
				memcpy(cbAllPlayerWeaveItemArray[itChair->first][k].cbCardData, itChair->second.weaveItem[k].cbCardData,
					   sizeof(itChair->second.weaveItem[k].cbCardData));
				cbAllPlayerWeaveItemArray[itChair->first][k].wProvideUser = itChair->second.weaveItem[k].wProvideUser;
				cbAllPlayerWeaveItemArray[itChair->first][k].cbPublicCard = itChair->second.weaveItem[k].cbPublicCard;
				cbAllPlayerWeaveItemArray[itChair->first][k].cbCenterCard = itChair->second.weaveItem[k].cbCenterCard;
				if (itChair->second.weaveItem[k].cbWeaveKind == WIK_LEFT_MY) {
					cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_LEFT;
				} else if (itChair->second.weaveItem[k].cbWeaveKind == WIK_CENTER_MY) {
					cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_CENTER;
				} else if (itChair->second.weaveItem[k].cbWeaveKind == WIK_RIGHT_MY) {
					cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_RIGHT;
				} else if (itChair->second.weaveItem[k].cbWeaveKind == WIK_PENG_MY) {
					cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_PENG;
				} else if (itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_AN_MY ||
						   itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_MY ||
						   itChair->second.weaveItem[k].cbWeaveKind == WIK_GANG_MING_PENG_MY) {
					cbAllPlayerWeaveItemArray[itChair->first][k].cbWeaveKind = WIK_GANG;
				}
				cbWeaveItemCount++;
			}
			cbAllPlayerWeaveItemCount[itChair->first] = cbWeaveItemCount;
		}
		CT_CHAR szGameRoomName[LEN_SERVER] = "test";
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		CT_BYTE cbTing = m_GameLogic.AnalyseTingCard(dwChairID, m_dwBanker, cbAllPlayerHandCardIndex,
													 cbAllPlayerDiscardCard, cbAllPlayerDiscardCount,
													 m_cbAllPlayerOutCardCount,
													 cbAllPlayerWeaveItemArray, cbAllPlayerWeaveItemCount,
													 cbAllPlayerListenStatus, bAllPlayerIsTianTing, 0,
													 GetMenFeng(dwChairID),
													 m_cbLeftCardCount, m_bGangStatus, szGameRoomName,
													 it->second.cbListenCardData, it->second.cbListenCardCount);
		if (cbTing == WIK_LISTEN)
		{
			it->second.cbPlayerAction |= WIK_TING_MY;
			/*CMD_S_OperateTip_Send data;
			data.wOperateCode = WIK_TING_MY;
			data.bGangCount = 0;
			memset(data.cbCardTB, 0, sizeof(data.cbCardTB));
			SendTableData(dwChairID, SUB_S_OPERATE_TIP_SEND, &data, sizeof(data));*/

			//LOG(ERROR)<<"==================Begin======================";
			//LOG(ERROR)<< "玩家:"<<it->second.dwUserID << " 能听牌";
			CMD_S_OutTip_Ting ting;
			CT_WORD  wLoop = it->second.cbListenCardCount > MAX_LISTEN_CARD_COUNT ? MAX_LISTEN_CARD_COUNT : it->second.cbListenCardCount;
			for(int i = 0; i < wLoop; i++)
			{
				tagListenCardData *pListen = (tagListenCardData *)it->second.cbListenCardData[i];
				if(pListen && pListen->cbOutCard != INVALID_CARD_DATA)
				{
					ting.bOutCardData[i] = pListen->cbOutCard;
					//LOG(ERROR) << "能出牌: "<< (int)ting.bOutCardData[i];
				}
			}
			//LOG(ERROR)<<"==================End======================";

			return CT_TRUE;
		}
	}

	return CT_FALSE;
}

CT_VOID CGameProcess::TipPlayerOutCard(CT_BYTE cbOutCardPlayer,CT_BYTE cbOutCardData, CT_BOOL bStartOutCardTimer, CT_BOOL bFirstTip/* = CT_FALSE*/)
{
	MapChairInfo::iterator it = m_mapChairInfo.find(cbOutCardPlayer);
	if(it == m_mapChairInfo.end())
	{
		LOG(ERROR) << "提示玩家出牌的时候，通过椅子ID ：" << cbOutCardPlayer << " 找不到玩家" << " 提示要出得牌是: "<< (int)cbOutCardData;
		return;
	}

	m_startOperateTime = Utility::GetTick();
	m_cbCurState = STATE_WAIT_OUT;
	m_dwShouldOutCardPlayer = cbOutCardPlayer;
    m_bCurOutCardPlayerFinish = CT_FALSE;
	m_dwAutoOutCardPlayer = cbOutCardPlayer;
	m_dWAutoOutCardData = cbOutCardData;;
	m_dwCurOperatePlayer = cbOutCardPlayer;
	if(bStartOutCardTimer)
	{
		//开启出牌倒计时
		if(bFirstTip)
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, TIMER_OUT_CARD * 1000);
		}
		else
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, TIMER_OUT_CARD * 1000);
		}
	}

	//发送出牌提示
	CMD_S_OutTip outTip;
	outTip.wOutCardUser = cbOutCardPlayer;
	outTip.bZhuangFirstOutCard = m_bZhuangFirstOutCard;
	for(CT_DWORD p = 0; p < GAME_PLAYER_MAX; p++)
	{
		if(p == m_dwBanker)
		{
			SendTableData(p,SUB_S_OUT_TIP, &outTip, sizeof(outTip), true);
		}
		else
		{
			SendTableData(p,SUB_S_OUT_TIP, &outTip, sizeof(outTip), false);
		}
	}
}

void CGameProcess::OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce)
{
    if (dwChairID == INVALID_CHAIR)
    {
        return;
    }

}

//设置指针
CT_VOID CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

    m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
}

//设置游戏黑名单
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{

}

//发送数据
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
	    LOG_IF(WARNING, PRINT_LOG_INFO) << "SendTableData: dwChairID = " << (int)dwChairID << ",wSubCmdID ="<< (int)wSubCmdID << " err";
		//LOG(WARNING)<< "SendTableData: dwChairID = " << (int)dwChairID << ",wSubCmdID ="<< (int)wSubCmdID << " err";
		return CT_FALSE;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}


