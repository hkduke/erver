#include "GameLogic.h"
#include <time.h>
#include <math.h>
#include "glog_wrapper.h"
#include <string.h>
#include <cmath>
#include <iostream>
#include <random>

//////////////////////////////////////////////////////////////////////////
//静态变量
CT_DOUBLE CGameLogic::m_sdColorPool = COLOR_POOL_INIT_AMOUNR;						//彩池
CT_DOUBLE CGameLogic::m_sdRealColorPool = 0.0;										//真人彩池
CT_DOUBLE CGameLogic::m_sdRobotColorPool = 0.0;										//机器人彩池
CT_DOUBLE CGameLogic::m_sllRealOutColorPool = 0;
CT_LONGLONG CGameLogic::m_sllHBAmount[MAX_DISPLAY_HB_COUNT]={0};

//////////////////////////////////////////////////////////////////////////


//构造函数
CGameLogic::CGameLogic():m_gen(m_rd()), m_iProba(0,99)
{
    srand(time(NULL));
    m_dwRandIndex = 0;
    m_bCtrlMasterSwitch = false;
}

//析构函数
CGameLogic::~CGameLogic()
{
}
bool CGameLogic::RandAssignHB(tagHBInfo & stHBInfo)
{
    //先最小分配确保分成固定数量的红包
    CT_LONGLONG llSum = 0;
    m_sllHBAmount[stHBInfo.stFaHBInfo.dwHBID%MAX_DISPLAY_HB_COUNT] = stHBInfo.stFaHBInfo.iHBAmount*TO_SCORE;
    stHBInfo.stAssignHBInfo.wRemainHBCount = stHBInfo.stFaHBInfo.iHBAllotCount;
    stHBInfo.stAssignHBInfo.llRemainAmount = stHBInfo.stAssignHBInfo.llTotalAmount - llSum;
    m_llHBAverageAmount = 0;
    int iCount = 0;
    for (int i = 0; i < MAX_DISPLAY_HB_COUNT; ++i)
    {
        if(0 == m_sllHBAmount[i])
        {
            continue;
        }
        m_llHBAverageAmount += m_sllHBAmount[i];
        iCount++;
    }
    m_llHBAverageAmount /= iCount;
    return true;
}

CT_INT32 CGameLogic::RandAssignHB2(tagHBInfo & stHBInfo, CT_INT32 iGrabHBPlayerCtrlType, CT_BOOL bIsRobot, CT_BOOL bIsFaRobot, CT_BOOL bIsSelfFaSelfGrab, CT_LONGLONG llGrabWinScore, GS_SimpleUserInfo & stGrabUserInfo)
{
    if(stHBInfo.stAssignHBInfo.wRemainHBCount <= 0)
    {
        return MAX_HB_COUNT;
    }

    int iExeProbaBase = 10;
    int iMaxThunderCount = 3;
    if(5 == stHBInfo.stFaHBInfo.iHBAllotCount)
    {
        iExeProbaBase = 5;
        iMaxThunderCount = 1;
    }
    else if(7 == stHBInfo.stFaHBInfo.iHBAllotCount)
    {
        iExeProbaBase = 8;
        iMaxThunderCount = 2;
    }
    int iPointCtrlGrabExeProba = iExeProbaBase*m_PointCtrlCfg.iGrabBaseProbaMultiple;
    if(iPointCtrlGrabExeProba < 0)
    {
        iPointCtrlGrabExeProba = 0;
    }
    else if(iPointCtrlGrabExeProba > 100)
    {
        iPointCtrlGrabExeProba = 100;
    }
    int i;
    CT_LONGLONG llScore = 0;
    CT_DWORD iWinPrizeIndex = INVALID_CHAIR;
    for (i = 0; i < stHBInfo.stFaHBInfo.iHBAllotCount; ++i)
    {
        if(stHBInfo.stAssignHBInfo.wChairID[i] != INVALID_CHAIR)
        {
            continue;
        }
        break;
    }

    if(stHBInfo.stAssignHBInfo.wRemainHBCount < 2 && i < stHBInfo.stFaHBInfo.iHBAllotCount)
    {
        stHBInfo.stAssignHBInfo.llOutHB[i] = stHBInfo.stAssignHBInfo.llRemainAmount;
        stHBInfo.stAssignHBInfo.llRemainAmount = 0;
        iWinPrizeIndex = JudgeWinPrize(stHBInfo, i);
    }
    else
    {
        std::uniform_int_distribution<unsigned int> disValue((int)(stHBInfo.stAssignHBInfo.llRemainAmount/stHBInfo.stAssignHBInfo.wRemainHBCount*0.1), (int)(stHBInfo.stAssignHBInfo.llRemainAmount/stHBInfo.stAssignHBInfo.wRemainHBCount*1.9));
        llScore = disValue(m_gen);
        //llScore = RAND((CT_UINT32)(stHBInfo.stAssignHBInfo.llRemainAmount/stHBInfo.stAssignHBInfo.wRemainHBCount*0.1),(CT_UINT32)(stHBInfo.stAssignHBInfo.llRemainAmount/stHBInfo.stAssignHBInfo.wRemainHBCount*1.9),m_dwRandIndex++);
        //int iRand = RAND_NUM(m_dwRandIndex++)%(100);
        //std::uniform_int_distribution<unsigned int> disRand(0,100);
        //int iRand = disRand(m_gen);
        int iTemp = stHBInfo.stAssignHBInfo.llOutHB[i]+llScore;
        CT_BOOL bIsCtrl = true;
        if(!m_bCtrlMasterSwitch)
        {
            bIsCtrl = false;
        }
        else if((en_NoCtrl == iGrabHBPlayerCtrlType) && (en_NoCtrl == stHBInfo.stAssignHBInfo.iStockCtrlType) && (en_NoCtrl==stHBInfo.stAssignHBInfo.iFaPointCtrlType))
        {
            /*if(((m_llHBAverageAmount*15) < llGrabWinScore) && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) && (iRand < 30)&& (stHBInfo.stAssignHBInfo.iThunderCount < iMaxThunderCount))
            {
                int iTempThunder = iTemp % 10;
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);

                //LOG(ERROR)<< "< m_llHBAverageAmount:" << m_llHBAverageAmount << " llGrabWinScore:" << llGrabWinScore << " iRand:" << iRand;
            }
            else if(((m_llHBAverageAmount*15*-1) > llGrabWinScore) && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO) && (iRand < 30))
            {
                llScore += 1;
                //LOG(ERROR)<< "> m_llHBAverageAmount:" << m_llHBAverageAmount << " llGrabWinScore:" << llGrabWinScore << " iRand:" << iRand;
            }
            else*/
            {
                bIsCtrl = false;
            }
            //不需要做某事
        }
        else if(bIsFaRobot)
        {
            int iRobatFaProb = 0;
            if(5==stHBInfo.stFaHBInfo.iHBAllotCount)
            {
                iRobatFaProb = m_StockKillScoreCfg.i5HBLevelRobotFaRealThunderProba;
            }
            else if(7==stHBInfo.stFaHBInfo.iHBAllotCount)
            {
                iRobatFaProb = m_StockKillScoreCfg.i7HBLevelRobotFaRealThunderProba;
            }
            else
            {
                iRobatFaProb = m_StockKillScoreCfg.i10HBLevelRobotFaRealThunderProba;
            }
            int iTempThunder = iTemp % 10;
            //机器人发红包真人抢红包，系统吃分
            if((!bIsRobot) && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) &&(en_EatScore == stHBInfo.stAssignHBInfo.iStockCtrlType) && ((int)m_iProba(m_gen) < iRobatFaProb)
            && (en_PlayerEatScore != iGrabHBPlayerCtrlType) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
            {
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
            }
                //机器人发红包真人抢红包，系统吐分
            else if((!bIsRobot) && (en_OutScore == stHBInfo.stAssignHBInfo.iStockCtrlType) && (en_PlayerOutScore != iGrabHBPlayerCtrlType) && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO)
            && (m_iProba(m_gen) < 70) && (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
            {
                llScore += 1;
            }
            //机器人发红包真人抢红包，点控玩家吐分
            else if((en_PlayerOutScore == iGrabHBPlayerCtrlType) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba) && (en_OutScore != stHBInfo.stAssignHBInfo.iStockCtrlType)
            && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
            {
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
            }
            //机器人发红包真人抢红包，点控玩家吃分
            else if((en_PlayerEatScore == iGrabHBPlayerCtrlType) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba) && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO)
            && (en_EatScore != stHBInfo.stAssignHBInfo.iStockCtrlType)&& (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
            {
                llScore += 1;
            }
            else
            {
                bIsCtrl = false;
            }
        }
        else if(!bIsFaRobot)
        {
            int iTempThunder = iTemp % 10;
            //真人发红包机器人抢红包，系统吃分
            if((bIsRobot) && (en_EatScore == stHBInfo.stAssignHBInfo.iStockCtrlType) && (en_PlayerEatScore != stHBInfo.stAssignHBInfo.iFaPointCtrlType)
            && ((int)m_iProba(m_gen) > m_StockKillScoreCfg.iRealFaHBRobotThunderProba) && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO)&& (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
            {
                llScore += 1;
            }
            //真人发红包机器人抢红包，系统吐分
            else if((bIsRobot) && (en_OutScore == stHBInfo.stAssignHBInfo.iStockCtrlType) && (en_PlayerOutScore != stHBInfo.stAssignHBInfo.iFaPointCtrlType) && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO)
            && (int)m_iProba(m_gen) < m_StockKillScoreCfg.iSysOutScorRealFaHBRobotThunderProba && (stHBInfo.stAssignHBInfo.iRobotThunderCount <= 2) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
            {
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
            }
            //真人发红包真人抢红包，点控玩家吐分
            else if((en_PlayerOutScore == iGrabHBPlayerCtrlType) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba)
            && (en_PlayerOutScore != stHBInfo.stAssignHBInfo.iFaPointCtrlType) && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
            {
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
            }
            //真人发红包真人抢红包，点控玩家吃分
            else if((en_PlayerEatScore == iGrabHBPlayerCtrlType) && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba)
            && (en_PlayerEatScore != stHBInfo.stAssignHBInfo.iFaPointCtrlType) && (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
            {
                llScore += 1;
            }
            //发红包玩家吐分
            else if((en_PlayerOutScore == stHBInfo.stAssignHBInfo.iFaPointCtrlType) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba)
            && (iTemp % 10 == stHBInfo.stFaHBInfo.cbThunderNO) && (en_PlayerOutScore != iGrabHBPlayerCtrlType) && (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
            {
                llScore += 1;
            }
            //发红包玩家吃分
            else if((en_PlayerEatScore == stHBInfo.stAssignHBInfo.iFaPointCtrlType) && ((int)m_iProba(m_gen) < iPointCtrlGrabExeProba)
            && (stHBInfo.stAssignHBInfo.iThunderCount < iMaxThunderCount) && (en_PlayerEatScore != iGrabHBPlayerCtrlType)
            && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
            {
                llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
            }
            else
            {
                bIsCtrl = false;
            }

        }
        //LOG(ERROR) << "bIsCtrl:" << (int)bIsCtrl << " isRobot:" << (int)bIsRobot << " llScore:" << llScore;
        //iRand = RAND_NUM(m_dwRandIndex++)%(100);
        int iTempThunder = iTemp % 10;
        //防止控制是单个红包雷数太多
        if((bIsCtrl) && (!bIsRobot) && (en_PlayerOutScore != iGrabHBPlayerCtrlType) && (stHBInfo.stAssignHBInfo.iThunderCount > iMaxThunderCount)
        && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO) && (m_iProba(m_gen) < 50) && (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
        {
            llScore += 1;
        }
        //防止连续赢
        else if((!bIsRobot) && (stGrabUserInfo.llGrabContinueWinScore > (m_llHBAverageAmount*4)) && (stGrabUserInfo.llWinLoseScore > 0)
        && (m_iProba(m_gen) < 60) && ((iTemp % 10) != stHBInfo.stFaHBInfo.cbThunderNO) && (stHBInfo.stAssignHBInfo.llRemainAmount > (iTempThunder+10)))
        {
            llScore += (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);
        }
        //防止连续输
        else if((!bIsRobot) && (stGrabUserInfo.llGrabContinueLoseScore < (m_llHBAverageAmount*2*-1)) && (m_iProba(m_gen) < 60)
        && ((iTemp % 10) == stHBInfo.stFaHBInfo.cbThunderNO) && (stHBInfo.stAssignHBInfo.llRemainAmount > 10))
        {
            llScore += 1;
        }

        //iRand = RAND_NUM(m_dwRandIndex++)%(100);
        //彩池控制
        for (CT_DWORD j = 0; j < m_vColorPoolCfg.size(); ++j)
        {
            iTemp = llScore+stHBInfo.stAssignHBInfo.llOutHB[i];
            std::string strTmp = std::to_string(iTemp);
            for (CT_DWORD k = 0; k < m_vColorPoolCfg[j].vWinPrizeNumber.size(); ++k)
            {
                std::string strWinPrizeNumber = std::to_string(m_vColorPoolCfg[j].vWinPrizeNumber[k]);
                if((strTmp.length() >= strWinPrizeNumber.length()) && ((int)m_iProba(m_gen) <  m_vColorPoolCfg[j].iWinPrizeProba) &&
                (0 == strncmp(strWinPrizeNumber.c_str(),strTmp.c_str()+(strTmp.length()-strWinPrizeNumber.length()),strWinPrizeNumber.length())) &&
                   ((bIsRobot && (int)m_iProba(m_gen) < m_vColorPoolCfg[j].iRobotWinPrizeProba)
                    || ((!bIsRobot) && (en_PlayerOutScore == iGrabHBPlayerCtrlType) && (int)m_iProba(m_gen) < m_vColorPoolCfg[j].iPointCtlLoseWinProba)
                    || (((!bIsRobot) && en_PlayerOutScore != iGrabHBPlayerCtrlType))))
                {
                    iWinPrizeIndex = j;
                }
                else if(strTmp.length() >= strWinPrizeNumber.length() && (0 == strncmp(strWinPrizeNumber.c_str(),strTmp.c_str()+(strTmp.length()-strWinPrizeNumber.length()),strWinPrizeNumber.length())))
                {
                    llScore += 1;
                }
            }
            CT_LONGLONG llTotalAmount = (CT_LONGLONG)m_sdColorPool * (m_vColorPoolCfg[j].iFaWinPrizeRatio+m_vColorPoolCfg[j].iGrabWinPrizeRatio)/100;
            if((iWinPrizeIndex == j) && ((!bIsRobot)||(!bIsFaRobot)) && (iWinPrizeIndex != INVALID_CHAIR) &&
               ((m_sdRealColorPool < (m_vColorPoolCfg[j].llMinWinPrizeAmount*2)) || llTotalAmount > m_sdRealColorPool))
            {

                llScore += 1;
                iWinPrizeIndex = INVALID_CHAIR;
            }
            else if((iWinPrizeIndex == j) && ((bIsRobot)&&(bIsFaRobot)) && (iWinPrizeIndex != INVALID_CHAIR) && (llTotalAmount > m_sdRobotColorPool))
            {
                llScore += 1;
                iWinPrizeIndex = INVALID_CHAIR;
            }
        }

        stHBInfo.stAssignHBInfo.llOutHB[i] += llScore;
        stHBInfo.stAssignHBInfo.llRemainAmount -= llScore;
    }

    CalculateColorPrize(stHBInfo, i, iWinPrizeIndex, bIsRobot, bIsFaRobot);

    if(stHBInfo.stAssignHBInfo.llOutHB[i] && (stHBInfo.stAssignHBInfo.llOutHB[i]%10 == stHBInfo.stFaHBInfo.cbThunderNO))
    {
        if(!bIsSelfFaSelfGrab)
        {
            stHBInfo.stAssignHBInfo.llThunderBackAmount[i] = stHBInfo.stFaHBInfo.dMultiple*stHBInfo.stAssignHBInfo.llTotalAmount;
        }
        stHBInfo.stAssignHBInfo.bIsThunder[i] = true;
        stHBInfo.stAssignHBInfo.iThunderCount++;
        if(bIsRobot)
        {
            stHBInfo.stAssignHBInfo.iRobotThunderCount++;
        }
    }
    stHBInfo.stAssignHBInfo.wRemainHBCount--;
    if(bIsRobot)
    {
        stHBInfo.stAssignHBInfo.iRobotGrabCount++;
    }
    return i;
}
CT_DWORD CGameLogic::JudgeWinPrize(tagHBInfo & stHBInfo, int i)
{
    int iTemp = stHBInfo.stAssignHBInfo.llOutHB[i];

    int iWinPrizeIndex = INVALID_CHAIR;
    //彩池
    for (CT_DWORD j = 0; j < m_vColorPoolCfg.size(); ++j)
    {
        std::string strTmp = std::to_string(iTemp);
        for (CT_DWORD k = 0; k < m_vColorPoolCfg[j].vWinPrizeNumber.size(); ++k)
        {
            std::string strWinPrizeNumber = std::to_string(m_vColorPoolCfg[j].vWinPrizeNumber[k]);
            if(strTmp.length() >= strWinPrizeNumber.length() && (0 == strncmp(strWinPrizeNumber.c_str(),strTmp.c_str()+(strTmp.length()-strWinPrizeNumber.length()),strWinPrizeNumber.length())))
            {
                iWinPrizeIndex = j;
            }
        }
    }

    return iWinPrizeIndex;
}
CT_VOID CGameLogic::CalculateColorPrize(tagHBInfo & stHBInfo, int i, CT_DWORD iWinPrizeIndex, CT_BOOL bIsRobot, CT_BOOL bIsFaRobot)
{
    if(iWinPrizeIndex != INVALID_CHAIR)
    {
        CT_LONGLONG llTotalAmount = (CT_LONGLONG)m_sdColorPool * (m_vColorPoolCfg[iWinPrizeIndex].iFaWinPrizeRatio+m_vColorPoolCfg[iWinPrizeIndex].iGrabWinPrizeRatio)/100;
        stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i] = llTotalAmount * m_vColorPoolCfg[iWinPrizeIndex].iGrabWinPrizeRatio/(m_vColorPoolCfg[iWinPrizeIndex].iFaWinPrizeRatio+m_vColorPoolCfg[iWinPrizeIndex].iGrabWinPrizeRatio);
        CT_LONGLONG llFaAmount = llTotalAmount * m_vColorPoolCfg[iWinPrizeIndex].iFaWinPrizeRatio/(m_vColorPoolCfg[iWinPrizeIndex].iFaWinPrizeRatio+m_vColorPoolCfg[iWinPrizeIndex].iGrabWinPrizeRatio);
        stHBInfo.stAssignHBInfo.llFaWinColorPoolAmount += llFaAmount;
        llTotalAmount = llFaAmount + stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i];
        if(bIsRobot)
        {
            m_sdRobotColorPool -= stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i];
        }
        else
        {
            m_sdRealColorPool -= stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i];
            m_sllRealOutColorPool += stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i];
        }
        if(bIsFaRobot)
        {
            m_sdRobotColorPool -= llFaAmount;
        }
        else
        {
            m_sdRealColorPool -= llFaAmount;
            m_sllRealOutColorPool += llFaAmount;
        }
        m_sdColorPool -= llTotalAmount;
        LOG(INFO) << stHBInfo.stFaHBInfo.dwHBID <<" bIsRobot:" << (int)bIsRobot << " Amount:" << stHBInfo.stAssignHBInfo.llOutHB[i]
                  <<" m_sdRealColorPool: " << m_sdRealColorPool << " m_sdColorPool:" << m_sdColorPool<<" m_sdRobotColorPool: " << m_sdRobotColorPool <<" llGrabWinColorPoolAmount: " << stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[i];
    }

    if(m_sdColorPool < COLOR_POOL_INIT_AMOUNR)
    {
        m_sdColorPool = COLOR_POOL_INIT_AMOUNR;
    }
    return;
}