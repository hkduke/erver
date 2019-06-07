//
// Created by luwei on 2018/5/28.
//
#include "stdafx.h"
#include "ActivityMgr.h"
#include <timeFunction.h>
#include "NetModule.h"
#include "ServerMgr.h"
#include "SystemMsgMgr.h"
#include <Utility.h>
#include <math.h>

extern CNetConnector *pNetDB;
bool comp_by_user_score(tagWealthGodComingUserInfo& p1, tagWealthGodComingUserInfo& p2)
{
    return p1.dwScore > p2.dwScore;
}

CActivityMgr::CActivityMgr()
:m_bActivityStatusChange(true)
,m_bDoingActivityStatusChange(true)
{

}

CActivityMgr::~CActivityMgr()
{

}

void CActivityMgr::InsertWealthGodComing(tagWealthGodComingCfg* pWealthGodComing)
{
    //加载配置重复
    auto it = m_mapWealthGodComingCfg.find(pWealthGodComing->wID);
    if (it != m_mapWealthGodComingCfg.end())
    {
        LOG(WARNING) << "insert wealth god coming cfg fail. repeated cfg! id: " << pWealthGodComing->wID;
        return;
    }
    
    //配置1
    m_mapWealthGodComingCfg[pWealthGodComing->wID] = *pWealthGodComing;
    
    //配置2
    CT_DWORD dwGameIndex = pWealthGodComing->wGameID*100+pWealthGodComing->wKindID*10+pWealthGodComing->wRoomKindID;
    std::vector<tagWealthGodComingCfg>& vecWealthGodComingCfg = m_mapWealthGodComingCfg2[dwGameIndex];
    vecWealthGodComingCfg.push_back(*pWealthGodComing);
    
    std::sort(vecWealthGodComingCfg.begin(), vecWealthGodComingCfg.end(), [](tagWealthGodComingCfg& a, tagWealthGodComingCfg& b) {
        return a.cbStartHour < b.cbStartHour;
    });
}

void CActivityMgr::InsertWealthGodComingReward(tagWealthGodComingReward* pReward)
{
   // m_mapWealthGodComingReward[pReward->cbRank] = *pReward;
    
    CT_DWORD dwGameid = pReward->wGameID*100+pReward->wKindID*10+pReward->wRoomKindID;
    std::map<CT_BYTE, tagWealthGodComingReward>& mapReward = m_mapWealthGodComingReward[dwGameid];
    mapReward[pReward->cbRank] = *pReward;
    
    //LOG(WARNING) << "load wealth god coming reward, gamd index : " << dwGameid ;
}

void CActivityMgr::UpdateWealthGodComing(tagWealthGodComingCfg* pWealthGodComing)
{
    m_bActivityStatusChange = true;

    //如果是新插入数据
    auto it = m_mapWealthGodComingCfg.find(pWealthGodComing->wID);
    if (it == m_mapWealthGodComingCfg.end())
    {
        InsertWealthGodComing(pWealthGodComing);

        //
        CT_DWORD dwGameIndex = pWealthGodComing->wGameID*100+pWealthGodComing->wKindID*10+pWealthGodComing->wRoomKindID;
        auto itIndex = m_mapWealthGodComingLastIndex.find(dwGameIndex);
        if (itIndex == m_mapWealthGodComingLastIndex.end())
        {
            tagWealthGodComingLastIndex lastIndex;
            lastIndex.wGameID = pWealthGodComing->wGameID;
            lastIndex.wKindID = pWealthGodComing->wKindID;
            lastIndex.wRoomKindID = pWealthGodComing->wRoomKindID;
            lastIndex.dwLastIndex = 0;
            m_mapWealthGodComingLastIndex.insert(std::make_pair(dwGameIndex, lastIndex));
        }

        LOG(WARNING) << "update wealth god coming, insert new cfg, id: " << pWealthGodComing->wID << ", status: " << (int)pWealthGodComing->cbState;
        return;
    }
    else
    {
        //配置1
        m_mapWealthGodComingCfg[pWealthGodComing->wID] = *pWealthGodComing;
    
        //配置2
        CT_DWORD dwGameIndex = pWealthGodComing->wGameID*100+pWealthGodComing->wKindID*10+pWealthGodComing->wRoomKindID;
        std::vector<tagWealthGodComingCfg>& vecWealthGodComingCfg = m_mapWealthGodComingCfg2[dwGameIndex];
        for (auto itCfg = vecWealthGodComingCfg.begin(); itCfg != vecWealthGodComingCfg.end(); ++itCfg)
        {
            if (itCfg->wID == pWealthGodComing->wID)
            {
                vecWealthGodComingCfg.erase(itCfg);
                break;
            }
        }
        
        vecWealthGodComingCfg.push_back(*pWealthGodComing);
        std::sort(vecWealthGodComingCfg.begin(), vecWealthGodComingCfg.end(), [](tagWealthGodComingCfg& a, tagWealthGodComingCfg& b) {
          return a.cbStartHour < b.cbStartHour;
        });

        LOG(WARNING) << "update wealth god coming, id: " << pWealthGodComing->wID << ", status: " << (int)pWealthGodComing->cbState;
    }
}

void CActivityMgr::InsertWealthGodComingIndex(tagWealthGodComingLastIndex* pIndex)
{
    CT_DWORD dwKey = pIndex->wGameID * 100 + pIndex->wKindID * 10 + pIndex->wRoomKindID;
    m_mapWealthGodComingLastIndex[dwKey] = *pIndex;
}

void CActivityMgr::InsertWealthGodComingHistoryData(tagWealthGodComingData* pWealthGodComingData)
{
    CT_WORD wActivityID = pWealthGodComingData->wID;
    auto itCfg = m_mapWealthGodComingCfg.find(wActivityID);
    if (itCfg == m_mapWealthGodComingCfg.end())
    {
        return;
    }
    
    CT_DWORD dwGameIndex = itCfg->second.wGameID*100 + itCfg->second.wKindID*10 + itCfg->second.wRoomKindID;
    std::map<CT_DWORD, tagWealthGodComingData>& mapData= m_mapHistoryWealthGodComingData[dwGameIndex];
    mapData.insert(std::make_pair(pWealthGodComingData->dwIndex, *pWealthGodComingData));
}

void CActivityMgr::InsertWealthGodComingHistoryUser(tagWealthGodComingUserInfoEx* pWealthGodComingUser)
{
    CT_DWORD dwGameIndex = pWealthGodComingUser->wGameID * 100 + pWealthGodComingUser->wKindID * 10 + pWealthGodComingUser->wRoomKindID;
    std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo>>& mapUserInfo = m_mapHistoryWealthGodComingUserInfo[dwGameIndex];
    std::vector<tagWealthGodComingUserInfo>& vecUserInfo = mapUserInfo[pWealthGodComingUser->dwIndex];
    for (auto& it : vecUserInfo)
    {
        if (it.dwUserID == pWealthGodComingUser->dwUserID)
            return;
    }

    tagWealthGodComingUserInfo HistoryUserInfo;
    HistoryUserInfo.dwUserID = pWealthGodComingUser->dwUserID;
    HistoryUserInfo.dwIndex = pWealthGodComingUser->dwIndex;
    HistoryUserInfo.dwScore = pWealthGodComingUser->dwScore;
    HistoryUserInfo.cbVip2 = pWealthGodComingUser->cbVip2;
    HistoryUserInfo.cbGender = pWealthGodComingUser->cbGender;
    HistoryUserInfo.cbHeadIndex = pWealthGodComingUser->cbHeadIndex;
    HistoryUserInfo.wCompleteCount = pWealthGodComingUser->wCompleteCount;
    HistoryUserInfo.dwRevenue = pWealthGodComingUser->dwRevenue;
    HistoryUserInfo.dwRewardScore = pWealthGodComingUser->dwRewardScore;
    HistoryUserInfo.cbRank = pWealthGodComingUser->cbRank;
    _snprintf_info(HistoryUserInfo.szNickName, sizeof(HistoryUserInfo.szNickName), "%s", pWealthGodComingUser->szNickName);
    vecUserInfo.push_back(HistoryUserInfo);
}

void CActivityMgr::CheckWealthGodComingStartEnd()
{
    tm timeNow;
	memset(&timeNow, 0, sizeof(timeNow));
	getLocalTime(&timeNow, time(NULL));
	
	bool bActivityChange = false;
	//判断有活动已经到
    for (auto it = m_mapDoingWealthGodComingData.begin(); it != m_mapDoingWealthGodComingData.end(); )
    {
        auto itCfg = m_mapWealthGodComingCfg.find(it->first);
        if (itCfg != m_mapWealthGodComingCfg.end())
        {
            tagWealthGodComingCfg& wealthGodComingCfg = itCfg->second;
            //if ((timeNow.tm_hour == wealthGodComingCfg.cbEndHour && timeNow.tm_min >= wealthGodComingCfg.cbEndMin)
            //  || (timeNow.tm_hour > wealthGodComingCfg.cbEndHour))
            if (!IsSpecialTime(timeNow,
                              wealthGodComingCfg.cbStartHour,
                              wealthGodComingCfg.cbStartMin,
                              wealthGodComingCfg.cbEndHour,
                              wealthGodComingCfg.cbEndMin))
            {
                CT_DWORD dwGameIndex = itCfg->second.wGameID*100+itCfg->second.wKindID*10+itCfg->second.wRoomKindID;
                
                //计算玩家排名
                CalWealthGodComingRank(it->second, dwGameIndex, itCfg->first);
                
                //删除索引值
                auto itDoingIndex = m_mapDoingWealthGodComingIndex.find(dwGameIndex);
                if (itDoingIndex != m_mapDoingWealthGodComingIndex.end())
                    m_mapDoingWealthGodComingIndex.erase(itDoingIndex);
                
                //删除正在进行的活动
                it = m_mapDoingWealthGodComingData.erase(it);
                
                if (!bActivityChange)
                    bActivityChange = true;
                continue;
            }
        }
        ++it;
    }
	
    //是否有新活动开始
    for (auto& it : m_mapWealthGodComingCfg)
    {
        //如果是无效状态，则不能开始
        if (it.second.cbState == 0)
            continue;
        
        //活动已经开始就不跳过
        auto itStart = m_mapDoingWealthGodComingData.find(it.first);
        if (itStart != m_mapDoingWealthGodComingData.end())
        {
            continue;
        }
        
        if (IsSpecialTime(timeNow, it.second.cbStartHour, it.second.cbStartMin, it.second.cbEndHour, it.second.cbEndMin))
        {
            //找到游戏的上一个场次信息
            CT_DWORD  dwGameIndex = it.second.wGameID * 100 + it.second.wKindID * 10 + it.second.wRoomKindID;
            auto itDoingIndex = m_mapDoingWealthGodComingIndex.find(dwGameIndex);
            if (itDoingIndex != m_mapDoingWealthGodComingIndex.end())
            {
                LOG(WARNING) << "this game has one wealth god activity. game index: " << dwGameIndex << ", activity id: " << it.second.wID;
                continue;
            }
            
            //检查本游戏的场次信息
            auto itLastIndex = m_mapWealthGodComingLastIndex.find(dwGameIndex);
            if (itLastIndex == m_mapWealthGodComingLastIndex.end())
            {
                continue;
            }
            
            itLastIndex->second.dwLastIndex += 1;
            
            tagWealthGodComingData wealthGodComingData;
            wealthGodComingData.wID = it.first;
            wealthGodComingData.dwIndex =  itLastIndex->second.dwLastIndex;
            _snprintf_info(wealthGodComingData.szDate, sizeof(wealthGodComingData.szDate), "%s", Utility::GetTimeNowString("%Y-%m-%d").c_str());
            _snprintf_info(wealthGodComingData.szTime, sizeof(wealthGodComingData.szTime), "%02d:%02d-%02d:%02d", it.second.cbStartHour, it.second.cbStartMin, it.second.cbEndHour, it.second.cbEndMin);
    
            //插入的正在进行队列
            m_mapDoingWealthGodComingData.insert(std::make_pair(it.first, wealthGodComingData));
            //LOG(WARNING) << "insert wealth god coming start, activity id: " << it.first;
            
            //插入正在进行队列索引
            m_mapDoingWealthGodComingIndex.insert(std::make_pair(dwGameIndex, it.first));
    
            //更新活动最后的索引到数据库
            UpdateActivityIndexTodb(&itLastIndex->second);
            //更新活动数据到数据库
            MSG_CS2DB_Insert_WGC_Data wgcData;
            wgcData.wGameID = it.second.wGameID;
            wgcData.wKindID = it.second.wKindID;
            wgcData.wRoomKind = it.second.wRoomKindID;
            wgcData.dwIndex = wealthGodComingData.dwIndex;
            wgcData.wActivityID = it.second.wID;
            wgcData.llRevenue = 0;
            _snprintf_info(wgcData.szStartTime, sizeof(wgcData.szStartTime), "%s %02d:%02d:00", Utility::GetTimeNowString("%Y-%m-%d").c_str(), it.second.cbStartHour, it.second.cbStartMin);
            _snprintf_info(wgcData.szEndTime, sizeof(wgcData.szEndTime), "%s %02d:%02d:00", Utility::GetTimeNowString("%Y-%m-%d").c_str(), it.second.cbEndHour, it.second.cbEndMin);
            InsertActivityData(&wgcData);
            
            //活动的用户信息
            std::map<CT_DWORD, tagWealthGodComingUserInfo> mapUserInfo;
            m_mapDoingWealthGodComingUserInfo[dwGameIndex] = mapUserInfo;
    
            if (!bActivityChange)
                bActivityChange = true;
        }
        else
        {
            //判断是5分钟以内广播信息
            int nNowResult = 100 * timeNow.tm_hour + timeNow.tm_min;
            int nStartResult = 100*it.second.cbStartHour + it.second.cbStartMin;
            if(nStartResult > nNowResult)
            {
                CT_DWORD dwLeftMinStart = (it.second.cbStartHour*60+it.second.cbStartMin - (timeNow.tm_hour*60 + timeNow.tm_min));
                if (dwLeftMinStart <= 1)
                {
                    std::string gameServerName =CServerMgr::get_instance().GetGameServerName(it.second.wGameID, it.second.wKindID, it.second.wRoomKindID);
                    
                    CT_CHAR szMsg[512] = {0};
                    _snprintf_info(szMsg, sizeof(szMsg), "距离新一轮财神奖励还有%d分钟，财神将降临：%s", dwLeftMinStart, gameServerName.c_str());
                    CSystemMsgMgr::get_instance().SendSystemMsg(0, szMsg);
                }
            }
        }
    }
    
    if (bActivityChange)
    {
        m_bActivityStatusChange = true;
        m_bDoingActivityStatusChange = true;
        SendDoingWealthGodComingToClient(0);
    }

}

void CActivityMgr::UserCheckDoingWealthGodComing(MSG_G2CS_PlayGame* pPlayGame)
{
    CT_DWORD dwGameIndex = pPlayGame->wGameID*100 + pPlayGame->wKindID * 10 + pPlayGame->wRoomKindID;
    
    //查找当前游戏是否有财神降临活动
    auto itDoingGameIndex = m_mapDoingWealthGodComingIndex.find(dwGameIndex);
    if (itDoingGameIndex == m_mapDoingWealthGodComingIndex.end())
        return;
    
    //查找活动ID
    CT_WORD  wActivityID = itDoingGameIndex->second;
    
    //活动的配置信息
    auto itCfg = m_mapWealthGodComingCfg.find(wActivityID);
    if (itCfg == m_mapWealthGodComingCfg.end())
        return;
    
    tagWealthGodComingCfg& wealthGodComingCfg = itCfg->second;
    
    //活动的汇总数据
    auto itDoingData = m_mapDoingWealthGodComingData.find(wActivityID);
    if (itDoingData == m_mapDoingWealthGodComingData.end())
        return;
    
    //更新活动的汇总数据
    tagWealthGodComingData& wealthGodComingData = itDoingData->second;
    if (pPlayGame->dwRevenue != 0)
    {
        wealthGodComingData.llRevenue += pPlayGame->dwRevenue;
        
        //更新活动data
        MSG_CS2DB_Update_WGC_Data wgcData;
        wgcData.wGameID = itCfg->second.wGameID;
        wgcData.wKindID = itCfg->second.wKindID;
        wgcData.wRoomKind = itCfg->second.wRoomKindID;
        wgcData.dwIndex = wealthGodComingData.dwIndex;
        wgcData.llRevenue = wealthGodComingData.llRevenue;
        UpdateActivetyData(&wgcData);
    }
    
    //检测正在进行的活动
    std::map<CT_DWORD, tagWealthGodComingUserInfo>& mapUserInfo = m_mapDoingWealthGodComingUserInfo[dwGameIndex];
    std::vector<tagWealthGodComingUserInfo*>& vecUserInfo = m_mapDoingWealthGodComingUserInfo2[dwGameIndex];
    auto itUser = mapUserInfo.find(pPlayGame->dwUserID);
    if (itUser == mapUserInfo.end())
    {
        /*tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pPlayGame->dwUserID);
        if (pUserInfo == NULL)
        {
            return;
        }*/
        if (pPlayGame->cbHeadID == 0)
        {
            tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pPlayGame->dwUserID);
            if (pUserInfo == NULL)
            {
                return;
            }
            pPlayGame->cbHeadID = pUserInfo->cbHeadID;
        }

        tagWealthGodComingUserInfo userInfo;
        userInfo.dwUserID = pPlayGame->dwUserID;
        userInfo.cbHeadIndex = pPlayGame->cbHeadID;
        userInfo.cbGender = pPlayGame->cbSex;
        _snprintf_info(userInfo.szNickName, sizeof(userInfo.szNickName), "%d", pPlayGame->dwUserID);
        userInfo.dwIndex = wealthGodComingData.dwIndex;
        
        //税收
        if (pPlayGame->dwRevenue != 0)
        {
            userInfo.dwRevenue += pPlayGame->dwRevenue;
        }
        
        //成绩
        if (wealthGodComingCfg.cbJudgeStander == en_WinLost_Judge)
        {
            if (pPlayGame->iWinScore > 0)
                userInfo.dwScore += pPlayGame->iWinScore;
            else if (pPlayGame->iWinScore < 0)
                userInfo.dwScore += -pPlayGame->iWinScore;//(CT_DWORD)ceil((-pPlayGame->iWinScore)*0.3);
        }
        else
        {
            if (pPlayGame->dwJettonScore > 0)
                userInfo.dwScore += pPlayGame->dwJettonScore;
        }
        
        userInfo.wCompleteCount += 1;
        mapUserInfo.insert(std::make_pair(pPlayGame->dwUserID, userInfo));
    
        //userinfo2
        vecUserInfo.push_back(&mapUserInfo[pPlayGame->dwUserID]);
    
        //更新玩家数据到DB
        MSG_CS2DB_Update_WGC_UserInfo userInfoTodb;
        userInfoTodb.wGameID = itCfg->second.wGameID;
        userInfoTodb.wKindID = itCfg->second.wKindID;
        userInfoTodb.wRoomKind = itCfg->second.wRoomKindID;
        userInfoTodb.dwIndex = wealthGodComingData.dwIndex;
        userInfoTodb.dwUserID = pPlayGame->dwUserID;
        userInfoTodb.dwRevenue = userInfo.dwRevenue;
        userInfoTodb.wCompleteCount = userInfo.wCompleteCount;
        userInfoTodb.dwScore = userInfo.dwScore;
        UpdateActivityUserInfo(&userInfoTodb);
    }
    else
    {
        tagWealthGodComingUserInfo& wealthGodComingUserInfo = itUser->second;
        
        //税收
        if (pPlayGame->dwRevenue != 0)
        {
            wealthGodComingUserInfo.dwRevenue += pPlayGame->dwRevenue;
        }
        
        //成绩
        if (wealthGodComingCfg.cbJudgeStander == en_WinLost_Judge)
        {
            if (pPlayGame->iWinScore > 0)
                wealthGodComingUserInfo.dwScore += pPlayGame->iWinScore;
            else if (pPlayGame->iWinScore < 0)
                wealthGodComingUserInfo.dwScore += -pPlayGame->iWinScore;//(CT_DWORD)ceil((-pPlayGame->iWinScore)*0.3);
        }
        else
        {
            if (pPlayGame->dwJettonScore > 0)
                wealthGodComingUserInfo.dwScore += pPlayGame->dwJettonScore;
        }
        
        wealthGodComingUserInfo.wCompleteCount += 1;
    
        //更新玩家数据到DB
        MSG_CS2DB_Update_WGC_UserInfo userInfoTodb;
        userInfoTodb.wGameID = itCfg->second.wGameID;
        userInfoTodb.wKindID = itCfg->second.wKindID;
        userInfoTodb.wRoomKind = itCfg->second.wRoomKindID;
        userInfoTodb.dwIndex = wealthGodComingData.dwIndex;
        userInfoTodb.dwUserID = pPlayGame->dwUserID;
        userInfoTodb.dwRevenue = wealthGodComingUserInfo.dwRevenue;
        userInfoTodb.wCompleteCount = wealthGodComingUserInfo.wCompleteCount;
        userInfoTodb.dwScore = wealthGodComingUserInfo.dwScore;
        UpdateActivityUserInfo(&userInfoTodb);
    }
    
    //对玩家进行排名
    /*std::sort(vecUserInfo.begin(), vecUserInfo.end(), [](tagWealthGodComingUserInfo* a, tagWealthGodComingUserInfo* b)
    {
        return a->dwScore > b->dwScore;
    });
    
    tm timeNow;
    memset(&timeNow, 0, sizeof(timeNow));
    getLocalTime(&timeNow, time(NULL));
    
    CT_DWORD dwTimeLeft = (itCfg->second.cbEndHour - timeNow.tm_hour)*60 + (itCfg->second.cbEndMin - timeNow.tm_min);
    MSG_SC_WealthGodComing_RealTimeInfo realTimeInfo;
    realTimeInfo.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    realTimeInfo.dwSubID = SUB_SC_WEALTHGODCOMING_REAL_TIME_INFO;
    realTimeInfo.dwTimeLeft = dwTimeLeft;
    realTimeInfo.wGameID = itCfg->second.wGameID;
    realTimeInfo.wKindID = itCfg->second.wKindID;
    realTimeInfo.wRoomKindID = itCfg->second.wRoomKindID;
    realTimeInfo.nTotalUser = vecUserInfo.size();
    int nRank = 0;
    for (auto& itVecUser : vecUserInfo)
    {
        realTimeInfo.nRank = ++nRank;
        acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(itVecUser->dwUserID);
        if (pSocket==NULL)
        {
            continue;
        }
    
        realTimeInfo.dwValue2 = itVecUser->dwUserID;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &realTimeInfo, sizeof(realTimeInfo));
    }*/
}

//计算活动玩家排名
void CActivityMgr::CalWealthGodComingRank(tagWealthGodComingData& wgcData, CT_DWORD dwGameIndex, CT_WORD wActivityID)
{
    //活动的配置信息
    auto itCfg = m_mapWealthGodComingCfg.find(wActivityID);
    if (itCfg == m_mapWealthGodComingCfg.end())
    {
        return;
    }
    
    //正在进行的活动的汇总数据
    auto itDoingData = m_mapDoingWealthGodComingData.find(wActivityID);
    if (itDoingData == m_mapDoingWealthGodComingData.end())
    {
        return;
    }
    
    //正在进行的活动的用户信息
    auto itDoingUserInfo = m_mapDoingWealthGodComingUserInfo.find(dwGameIndex);
    if (itDoingUserInfo == m_mapDoingWealthGodComingUserInfo.end())
    {
        LOG(WARNING) << "CalWealthGodComingRank no user info？ wActivityID id: " << wActivityID;
        return;
    }

    //读取信息
    std::map<CT_DWORD, tagWealthGodComingUserInfo>& mapWealthGodComingUserInfo = itDoingUserInfo->second;
    std::vector<tagWealthGodComingUserInfo> vecWealthGodComingUserInfo;
    for (auto& it : mapWealthGodComingUserInfo)
    {
        vecWealthGodComingUserInfo.push_back(it.second);
    }
    
    //删除正在进行活动的用户信息
    m_mapDoingWealthGodComingUserInfo.erase(itDoingUserInfo);
    
    auto itDoingUserInfo12 = m_mapDoingWealthGodComingUserInfo2.find(dwGameIndex);
    if (itDoingUserInfo12 != m_mapDoingWealthGodComingUserInfo2.end())
    {
        m_mapDoingWealthGodComingUserInfo2.erase(itDoingUserInfo12);
    }
    
    //用户排名
    std::sort(vecWealthGodComingUserInfo.begin(), vecWealthGodComingUserInfo.end(), comp_by_user_score);
    
    //奖励配置
    auto itReward = m_mapWealthGodComingReward.find(dwGameIndex);
    if (itReward == m_mapWealthGodComingReward.end())
    {
        LOG(WARNING) << "CalWealthGodComingRank can not find reward cfg. activity id: " << wActivityID;
        return;
    }
    
    //计算奖励信息
    std::map<CT_BYTE, tagWealthGodComingReward>& mapReward = itReward->second;
    /*CT_LONGLONG llTotalRewardScore = 0;
    for (auto& itMapReward : mapReward)
    {
        llTotalRewardScore += itMapReward.second.dwRewardScore;
    }*/

    //TODO: 临时屏蔽动态奖励
    /*CT_BOOL bUseDynamicReward = false;
    CT_LONGLONG llTotalDynamicReward = 0;
    if ((CT_LONGLONG)(wgcData.llRevenue*itCfg->second.wRewardRatio/100) >= llTotalRewardScore)
    {
        bUseDynamicReward = true;
        llTotalDynamicReward = (CT_LONGLONG)(wgcData.llRevenue*itCfg->second.wRewardRatio/100);
    }*/
    
    MSG_SC_WealthGodComing_End wealthGodComingEnd;
    wealthGodComingEnd.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    wealthGodComingEnd.dwSubID = SUB_SC_WEALTHGODCOMING_END;
    wealthGodComingEnd.wGameID = itCfg->second.wGameID;
    wealthGodComingEnd.wKindID = itCfg->second.wKindID;
    wealthGodComingEnd.wRoomKindID = itCfg->second.wRoomKindID;

    MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
    for (auto& it : mapAllGlobalUser)
    {
        acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
        if (pSocket==NULL)
        {
            continue;
        }

        wealthGodComingEnd.dwValue2 = it.first;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &wealthGodComingEnd, sizeof(wealthGodComingEnd));
    }
    
    //更新玩家的排名信息
    CT_BYTE cbRankCount = (CT_BYTE)mapReward.size();
    CT_BYTE nRank = 0;
    static CT_BYTE  szBuffer[SYS_NET_SENDBUF_CLIENT];
    CT_DWORD dwSendSize = 0;
    for (auto& it : vecWealthGodComingUserInfo)
    {
        //没到达到最低奖励人数不发奖励
        if (vecWealthGodComingUserInfo.size() < itCfg->second.wSendRewardMinUser)
        {
            it.cbRank = 255;
            it.dwRewardScore = 0;
            continue;
        }

        //通知活动结束
        /*acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.dwUserID);
        if (pSocket != NULL)
        {
            wealthGodComingEnd.dwValue2 = it.dwUserID;
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &wealthGodComingEnd, sizeof(wealthGodComingEnd));
        }*/

        //奖励前N名次
        if (nRank < cbRankCount && it.wCompleteCount >= itCfg->second.wCompleteCount)
        {
            it.cbRank = ++nRank;
            it.dwRewardScore = /*bUseDynamicReward ? (CT_DWORD)(llTotalDynamicReward*mapReward[it.cbRank].cbRewardRate/100) :*/ (CT_DWORD)mapReward[it.cbRank].dwRewardScore*TO_LL;
        }
        else
        {
            it.cbRank = 255;
            it.dwRewardScore = 0;
        }

        if (dwSendSize >= SYS_NET_SENDBUF_CLIENT)
        {
            //缓存爆满，先把部分数据发送到dbserver
            UpdateActivityAllUserRankInfo(szBuffer, dwSendSize);
            dwSendSize = 0;
        }

        static MSG_CS2DB_Update_WGC_UserInfo userInfoTodb;
        memset(&userInfoTodb, 0, sizeof(userInfoTodb));
        userInfoTodb.wGameID = itCfg->second.wGameID;
        userInfoTodb.wKindID = itCfg->second.wKindID;
        userInfoTodb.wRoomKind = itCfg->second.wRoomKindID;
        userInfoTodb.dwIndex = it.dwIndex;
        userInfoTodb.dwUserID = it.dwUserID;
        userInfoTodb.dwRevenue = it.dwRevenue;
        userInfoTodb.wCompleteCount = it.wCompleteCount;
        userInfoTodb.dwRewardScore = it.dwRewardScore;
        userInfoTodb.cbRank = it.cbRank;
        userInfoTodb.dwScore = it.dwScore;
        memcpy(szBuffer + dwSendSize, &userInfoTodb, sizeof(userInfoTodb));
        dwSendSize+= sizeof(userInfoTodb);
    }
    //把剩余的数据发送到dbserver
    if (dwSendSize > 0)
    {
        UpdateActivityAllUserRankInfo(szBuffer, dwSendSize);
    }

    //按排名进行重新排序
    std::sort(vecWealthGodComingUserInfo.begin(), vecWealthGodComingUserInfo.end(),  [](tagWealthGodComingUserInfo& a, tagWealthGodComingUserInfo& b) {
        return a.cbRank < b.cbRank;}
        );
    
    //派发奖励
    std::string gameServerName =CServerMgr::get_instance().GetGameServerName(itCfg->second.wGameID, itCfg->second.wKindID, itCfg->second.wRoomKindID);
    dwSendSize  = 0;
    
    CT_CHAR szChampionBoradcast[MAIL_CONTENT] = {0};
    for (auto& it : vecWealthGodComingUserInfo)
    {
        if (it.cbRank == 255 || it.cbRank == 0)
        {
            break;
        }
    
        if (dwSendSize >= SYS_NET_SENDBUF_CLIENT)
        {
            //缓存爆满，先把部分数据发送到dbserver
            SendRewardByMail(szBuffer, dwSendSize);
            dwSendSize = 0;
        }
        
        tagUserMail mail;
        mail.cbState = 0;
        mail.cbMailType = 2;
        mail.dwSendTime = Utility::GetTime();
        mail.dwExpiresTime = mail.dwSendTime + 89600 * 30; //默认30天的有效期
        mail.dwUserID = it.dwUserID;
        mail.llScore = it.dwRewardScore;
        mail.cbScoreChangeType = CSJL_REWARD;
        _snprintf_info(mail.szTitle, sizeof(mail.szTitle), "财神降临活动奖励");
        _snprintf_info(mail.szContent, sizeof(mail.szContent), "恭喜您在财神降临%s获得第%d名，奖励%.2f金币", gameServerName.c_str(),it.cbRank, it.dwRewardScore*TO_DOUBLE);
        //发送系统消息
        //CSystemMsgMgr::get_instance().SendSystemMsg(it.dwUserID, mail.szContent);
        //发送名次消息
        acl::aio_socket_stream* pUserProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(it.dwUserID);
        if (pUserProxySock)
        {
            MSG_SC_WealthGodComing_UserRank userRank;
            userRank.dwMainID = MSG_WEALTHGODCOMING_MAIN;
            userRank.dwSubID = SUB_SC_WEALTHGODCOMING_USER_RANK;
            userRank.dwValue2 = it.dwUserID;
            userRank.cbRank = it.cbRank;
            CNetModule::getSingleton().Send(pUserProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &userRank, sizeof(userRank));
        }
        
        //发送邮件消息提醒
        CUserMgr::get_instance().SendNewMailTip(it.dwUserID);
        
        //第一名的消息广播
        if (it.cbRank == 1)
        {
            _snprintf_info(szChampionBoradcast, sizeof(szChampionBoradcast), "恭喜%s在财神降临%s获得第%d名，奖励%.2f金币", it.szNickName, gameServerName.c_str(),it.cbRank, it.dwRewardScore*TO_DOUBLE);
        }
        
        memcpy(szBuffer + dwSendSize, &mail, sizeof(mail));
        dwSendSize+= sizeof(mail);
    }
    if (dwSendSize > 0)
    {
        SendRewardByMail(szBuffer, dwSendSize);
    }
    
    //第一名的消息广播
    if (strlen(szChampionBoradcast) > 0)
    {
        CSystemMsgMgr::get_instance().SendSystemMsg(0, szChampionBoradcast);
        //CSystemMsgMgr::get_instance().SendSystemMsg(0, szChampionBoradcast);
        //CSystemMsgMgr::get_instance().SendSystemMsg(0, szChampionBoradcast);
    }
    
    //历史活动的汇总数据
    std::map<CT_DWORD, tagWealthGodComingData>& mapHistoryWealthGodComingData = m_mapHistoryWealthGodComingData[dwGameIndex];
    tagWealthGodComingData& wealthGodComingData = itDoingData->second;
    mapHistoryWealthGodComingData.insert(std::make_pair(itDoingData->second.dwIndex, wealthGodComingData));
    
    //历史活动的排名数据
    std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo>>& mapHistoryUserInfo = m_mapHistoryWealthGodComingUserInfo[dwGameIndex];
    mapHistoryUserInfo.insert(std::make_pair(wealthGodComingData.dwIndex, vecWealthGodComingUserInfo));
    
    //检测历史排名信息是否超过10场次
    auto nWealthGodComingDataSize = mapHistoryWealthGodComingData.size();
    if (nWealthGodComingDataSize > 10)
    {
        //找到最早的活动场次
        std::vector<CT_DWORD > vecHistoryIndex;
        for (auto& it : mapHistoryWealthGodComingData)
        {
            vecHistoryIndex.push_back(it.first);
        }
        
        if (!vecHistoryIndex.empty())
        {
            std::sort(vecHistoryIndex.begin(), vecHistoryIndex.end(), [](CT_DWORD& a, CT_DWORD& b)
            {
              return a < b;
            });
    
            CT_DWORD dwEarliestIndex = vecHistoryIndex[0];
            mapHistoryWealthGodComingData.erase(dwEarliestIndex);
            mapHistoryUserInfo.erase(dwEarliestIndex);
        }
    }
}

void CActivityMgr::QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_WealthGodComing_Main))
    {
        return;
    }
    
    MSG_CS_WealthGodComing_Main* pQueryMain = (MSG_CS_WealthGodComing_Main*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryMain->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }
    
    static CT_CHAR     szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD    dwSendSize = 0;
    
    static MSG_GameMsgDownHead msgHead;
    memset(&msgHead, 0, sizeof(msgHead));
    msgHead.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    msgHead.dwSubID = SUB_SC_WEALTHGODCOMING_MAIN_INFO;
    msgHead.dwValue2 = pQueryMain->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);
   
    static std::vector<std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>> vecDoingActivity;
    static std::vector<std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>> vecNextActivity;
    static std::vector<std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>> vecEndActivity;
    
    if (m_bActivityStatusChange)
    {
        vecDoingActivity.clear();
        vecNextActivity.clear();
        vecEndActivity.clear();
        
        time_t rawtime;
        time(&rawtime);
        struct tm * timeinfo;
        timeinfo = localtime(&rawtime);
        for (auto& it : m_mapWealthGodComingCfg2)
        {
            std::vector<tagWealthGodComingCfg>& vecWealthGodComing = it.second;
            for (auto itVecCfg = vecWealthGodComing.begin(); itVecCfg != vecWealthGodComing.end(); ++itVecCfg)
            {
                if (itVecCfg->cbState == 0)
                    continue;
                    
                //检测是否正在进行的活动
                auto itDoing = m_mapDoingWealthGodComingData.find(itVecCfg->wID);
                if (itDoing != m_mapDoingWealthGodComingData.end())
                {
                    std::unique_ptr< MSG_SC_WealthGodComing_MainInfoEx> ptrMainInfo(new  MSG_SC_WealthGodComing_MainInfoEx);
                    ptrMainInfo->wActivityID = itVecCfg->wID;
                    ptrMainInfo->wGameID = itVecCfg->wGameID;
                    ptrMainInfo->wKindID = itVecCfg->wKindID;
                    ptrMainInfo->wRoomKind = itVecCfg->wRoomKindID;
                    _snprintf_info(ptrMainInfo->szTime, sizeof(ptrMainInfo->szTime), "%02d:%02d-%02d:%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin, itVecCfg->cbEndHour, itVecCfg->cbEndMin);
                    CT_CHAR szStartTime[TIME_LEN_2];
                    _snprintf_info(szStartTime, sizeof(szStartTime), "%02d%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin);
                    ptrMainInfo->wStartTime = (CT_WORD)atoi(szStartTime);
                    ptrMainInfo->cbStatus = 1;
                    vecNextActivity.push_back(std::move(ptrMainInfo));
                    //memcpy(szBuffer+dwSendSize, &mainInfo, sizeof(mainInfo));
                    // dwSendSize += sizeof(mainInfo);
                    break;
                }
            
                //检测下一场活动
                if (itVecCfg->cbStartHour > timeinfo->tm_hour || (itVecCfg->cbStartHour == timeinfo->tm_hour && itVecCfg->cbStartMin >= timeinfo->tm_min))
                {
                    std::unique_ptr< MSG_SC_WealthGodComing_MainInfoEx> ptrMainInfo(new  MSG_SC_WealthGodComing_MainInfoEx);
                    ptrMainInfo->wActivityID = itVecCfg->wID;
                    ptrMainInfo->wGameID = itVecCfg->wGameID;
                    ptrMainInfo->wKindID = itVecCfg->wKindID;
                    ptrMainInfo->wRoomKind = itVecCfg->wRoomKindID;
                    _snprintf_info(ptrMainInfo->szTime, sizeof(ptrMainInfo->szTime), "%02d:%02d-%02d:%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin, itVecCfg->cbEndHour, itVecCfg->cbEndMin);
                    CT_CHAR szStartTime[TIME_LEN_2];
                    _snprintf_info(szStartTime, sizeof(szStartTime), "%02d%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin);
                    ptrMainInfo->wStartTime = (CT_WORD)atoi(szStartTime);
                    ptrMainInfo->cbStatus = 0;
                    vecNextActivity.push_back(std::move(ptrMainInfo));
                    //memcpy(szBuffer+dwSendSize, &mainInfo, sizeof(mainInfo));
                    //dwSendSize += sizeof(mainInfo);
                    break;
                }
            
                //如果是这个游戏的最后一个活动了，说明都已经结束了
                if (itVecCfg == (vecWealthGodComing.end() - 1))
                {
                    std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx> ptrMainInfo(new MSG_SC_WealthGodComing_MainInfoEx);
                    ptrMainInfo->wActivityID = itVecCfg->wID;
                    ptrMainInfo->wGameID = itVecCfg->wGameID;
                    ptrMainInfo->wKindID = itVecCfg->wKindID;
                    ptrMainInfo->wRoomKind = itVecCfg->wRoomKindID;
                    _snprintf_info(ptrMainInfo->szTime, sizeof(ptrMainInfo->szTime), "%02d:%02d-%02d:%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin, itVecCfg->cbEndHour, itVecCfg->cbEndMin);
                    CT_CHAR szStartTime[TIME_LEN_2];
                    _snprintf_info(szStartTime, sizeof(szStartTime), "%02d%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin);
                    ptrMainInfo->wStartTime = (CT_WORD)atoi(szStartTime);
                    ptrMainInfo->cbStatus = 2;
                    vecEndActivity.push_back(std::move(ptrMainInfo));
                    //memcpy(szBuffer+dwSendSize, &mainInfo, sizeof(mainInfo));
                    //dwSendSize += sizeof(mainInfo);
                }
            }
        }
    
        std::sort(vecDoingActivity.begin(), vecDoingActivity.end(), [](std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& a, std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& b) {
          return a->wStartTime < b->wStartTime;
        });
    
        std::sort(vecNextActivity.begin(), vecNextActivity.end(), [](std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& a, std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& b) {
          return a->wStartTime < b->wStartTime;
        });
    
        //结束的按时间从小到大排列
        std::sort(vecEndActivity.begin(), vecEndActivity.end(), [](std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& a, std::unique_ptr<MSG_SC_WealthGodComing_MainInfoEx>& b) {
          return a->wStartTime > b->wStartTime;
        });
        
        m_bActivityStatusChange = false;
    }
    
    
    if (!vecDoingActivity.empty())
    {
        for (auto& it : vecDoingActivity)
        {
            memcpy(szBuffer+dwSendSize, it.get(), sizeof(MSG_SC_WealthGodComing_MainInfo));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_MainInfo);
        }
    }
    
    if (!vecNextActivity.empty())
    {
        for (auto& it : vecNextActivity)
        {
            memcpy(szBuffer+dwSendSize, it.get(), sizeof(MSG_SC_WealthGodComing_MainInfo));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_MainInfo);
        }
    }
    
    if (!vecEndActivity.empty())
    {
        for (auto& it : vecEndActivity)
        {
            memcpy(szBuffer+dwSendSize, it.get(), sizeof(MSG_SC_WealthGodComing_MainInfo));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_MainInfo);
        }
    }
    
    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CActivityMgr::QueryRewardRule(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_WealthGodComing_Reward_Rule))
    {
        return;
    }
    
    MSG_CS_WealthGodComing_Reward_Rule* pRewardRule = (MSG_CS_WealthGodComing_Reward_Rule*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pRewardRule->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }
    
    auto itCfg = m_mapWealthGodComingCfg.find(pRewardRule->wActivityID);
    if (itCfg == m_mapWealthGodComingCfg.end())
    {
        LOG(WARNING) << "QueryRewardRule can not find cfg, id: " << pRewardRule->wActivityID;
        return;
    }
    
    CT_DWORD dwGameIndex = itCfg->second.wGameID*100 + itCfg->second.wKindID*10 + itCfg->second.wRoomKindID;
    static CT_CHAR     szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD    dwSendSize = 0;
    static MSG_GameMsgDownHead msgHead;
    memset(&msgHead, 0, sizeof(msgHead));
    msgHead.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    msgHead.dwSubID = SUB_SC_WEALTHGODCOMING_REWARD_RULE;
    msgHead.dwValue2 = pRewardRule->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);
    
    auto itReward = m_mapWealthGodComingReward.find(dwGameIndex);
    if (itReward == m_mapWealthGodComingReward.end())
    {
        LOG(WARNING) << "QueryRewardRule can not find reward cfg. id : " << pRewardRule->wActivityID;
        return;
    }
    
    static MSG_SC_WealthGodComing_RewardRule rewardRule;
    std::map<CT_BYTE, tagWealthGodComingReward>& mapReward = itReward->second;
    
    //计算总奖励
    /*CT_LONGLONG llTotalRewardScore = 0;
    for (auto& itMapReward : mapReward)
    {
        llTotalRewardScore += itMapReward.second.dwRewardScore;
    }*/
    
    CT_BOOL bUseDynamicReward = false;
    //TODO: 临时屏蔽动态奖励
    /*CT_LONGLONG llTotalDynamicReward = 0;
    auto itDoingData = m_mapDoingWealthGodComingData.find(pRewardRule->wActivityID);
    if (itDoingData != m_mapDoingWealthGodComingData.end())
    {
        if ((CT_LONGLONG)(itDoingData->second.llRevenue*itCfg->second.wRewardRatio/100) >= llTotalRewardScore)
        {
            bUseDynamicReward = true;
            llTotalDynamicReward = itDoingData->second.llRevenue*itCfg->second.wRewardRatio/100;
        }
    }*/
    
    if (bUseDynamicReward)
    {
        for (auto& itMapReward : mapReward)
        {
            rewardRule.cbRank = itMapReward.first;
            rewardRule.wCompleteCount = itCfg->second.wCompleteCount;
            rewardRule.cbRankStandar = itCfg->second.cbJudgeStander;
            //rewardRule.dReward = (llTotalDynamicReward*itMapReward.second.cbRewardRate/100)*TO_DOUBLE;
            memcpy(szBuffer+dwSendSize, &rewardRule, sizeof(rewardRule));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_RewardRule);
        }
    }
    else
    {
        for (auto& itMapReward : mapReward)
        {
            rewardRule.cbRank = itMapReward.first;
            rewardRule.wCompleteCount = itCfg->second.wCompleteCount;
            rewardRule.cbRankStandar = itCfg->second.cbJudgeStander;
            rewardRule.dReward = itMapReward.second.dwRewardScore;
            memcpy(szBuffer+dwSendSize, &rewardRule, sizeof(rewardRule));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_RewardRule);
        }
    }
    
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CActivityMgr::QueryDetail(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize !=sizeof(MSG_CS_WealthGodComing_Detail))
    {
        return;
    }
    MSG_CS_WealthGodComing_Detail* pDetail = (MSG_CS_WealthGodComing_Detail*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pDetail->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }
    
    CT_DWORD dwGameIndex = pDetail->wGameID*100 + pDetail->wKindID*10 + pDetail->wRoomKind;
    
    static CT_CHAR     szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD    dwSendSize = 0;
    static MSG_GameMsgDownHead msgHead;
    memset(&msgHead, 0, sizeof(msgHead));
    msgHead.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    msgHead.dwSubID = SUB_SC_WEALTHGODCOMING_DETAIL;
    msgHead.dwValue2 = pDetail->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);
    
    static MSG_SC_WealthGodComing_Detail detail;
    
    //查询正在进行的活动
    CT_WORD wDoingActivityID = 0;
    auto itDoingIndex = m_mapDoingWealthGodComingIndex.find(dwGameIndex);
    if (itDoingIndex != m_mapDoingWealthGodComingIndex.end())
    {
        wDoingActivityID = itDoingIndex->second;
    }
    
    time_t rawtime;
    time(&rawtime);
    struct tm * timeinfo;
    timeinfo = localtime(&rawtime);
    
    //拷贝即将进行的活动
    auto itCfg2 = m_mapWealthGodComingCfg2.find(dwGameIndex);
    if (itCfg2 != m_mapWealthGodComingCfg2.end())
    {
        std::vector<tagWealthGodComingCfg>& vecWealthGodComing = itCfg2->second;
        for (auto itVecCfg = vecWealthGodComing.begin(); itVecCfg != vecWealthGodComing.end(); ++itVecCfg)
        {
            //如果是正在进行的活动，则跳过
            if (wDoingActivityID == itVecCfg->wID || itVecCfg->cbState == 0)
                continue;
        
            //检测下一场活动
            if ((itVecCfg->cbStartHour > timeinfo->tm_hour
              || (itVecCfg->cbStartHour==timeinfo->tm_hour && itVecCfg->cbStartMin >= timeinfo->tm_min)))
            {
                memset(&detail, 0 , sizeof(detail));
                detail.dwIndex = 0;
                detail.cbStatus = 0;
                _snprintf_info(detail.szDate, sizeof(detail.szDate), "%s", Utility::GetTimeNowString("%Y-%m-%d").c_str());
                _snprintf_info(detail.szTime, sizeof(detail.szTime), "%02d:%02d-%02d:%02d", itVecCfg->cbStartHour, itVecCfg->cbStartMin, itVecCfg->cbEndHour, itVecCfg->cbEndMin);
                memcpy(szBuffer + dwSendSize, &detail, sizeof(detail));
                dwSendSize += sizeof(detail);
                break;
            }
        }
    }
    
    //拷贝正在进行的活动
    if (wDoingActivityID != 0)
    {
        auto itDoing = m_mapDoingWealthGodComingData.find(wDoingActivityID);
        if (itDoing != m_mapDoingWealthGodComingData.end())
        {
            memset(&detail, 0 , sizeof(detail));
            detail.dwIndex = itDoing->second.dwIndex;
            detail.cbStatus = 1;
            _snprintf_info(detail.szDate, sizeof(detail.szDate), "%s", itDoing->second.szDate);
            _snprintf_info(detail.szTime, sizeof(detail.szTime), "%s", itDoing->second.szTime);
            auto itDoingGameUserInfo = m_mapDoingWealthGodComingUserInfo.find(dwGameIndex);
            if (itDoingGameUserInfo != m_mapDoingWealthGodComingUserInfo.end())
            {
                std::map<CT_DWORD, tagWealthGodComingUserInfo>& mapDoingUserInfo = itDoingGameUserInfo->second;
                auto itDoingUserInfo = mapDoingUserInfo.find(pDetail->dwUserID);
                if (itDoingUserInfo != mapDoingUserInfo.end())
                {
                    detail.cbSelfJoin = 1;
                    //LOG(WARNING) << "self join in doing.";
                }
            }
            memcpy(szBuffer + dwSendSize, &detail, sizeof(detail));
            dwSendSize += sizeof(detail);
        }
    }
    
    //拷贝历史活动
    auto itHistoryDoing = m_mapHistoryWealthGodComingData.find(dwGameIndex);
    if (itHistoryDoing != m_mapHistoryWealthGodComingData.end())
    {
        std::map<CT_DWORD, tagWealthGodComingData>& mapHistoryWealthGodComingData = itHistoryDoing->second;
        //for (auto& itHistoryDoingData : mapHistoryWealthGodComingData)
        std::vector<MSG_SC_WealthGodComing_Detail> vecWealthGodComingDetail;
        for (auto itHistoryDoingData = mapHistoryWealthGodComingData.begin(); itHistoryDoingData != mapHistoryWealthGodComingData.end(); ++itHistoryDoingData)
        {
            memset(&detail, 0 , sizeof(detail));
            detail.dwIndex = itHistoryDoingData->second.dwIndex;
            detail.cbStatus = 2;
            _snprintf_info(detail.szDate, sizeof(detail.szDate), "%s", itHistoryDoingData->second.szDate);
            _snprintf_info(detail.szTime, sizeof(detail.szTime), "%s", itHistoryDoingData->second.szTime);
            
            auto itHistoryGameUserInfo = m_mapHistoryWealthGodComingUserInfo.find(dwGameIndex);
            if (itHistoryGameUserInfo != m_mapHistoryWealthGodComingUserInfo.end())
            {
                std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo>>& mapHistoryUserInfo = itHistoryGameUserInfo->second;
                auto itMapHistoryUserInfo = mapHistoryUserInfo.find(itHistoryDoingData->second.dwIndex);
                if (itMapHistoryUserInfo != mapHistoryUserInfo.end())
                {
                    std::vector<tagWealthGodComingUserInfo>& vecUserInfo = itMapHistoryUserInfo->second;
                    for (auto& itVecHistoryUserInfo : vecUserInfo)
                    {
                        if (itVecHistoryUserInfo.dwUserID == pDetail->dwUserID)
                        {
                            detail.cbSelfJoin = 1;
                            //LOG(WARNING) << "self join in history.";
                            break;
                        }
                    }
                    
                    if (vecUserInfo.empty() || (!vecUserInfo.empty() && vecUserInfo[0].cbRank == 0))
                    {
                        detail.cbStatus = 3;
                    }
                }
                else
                {
                    detail.cbStatus = 3;
                }
            }
            else
            {
                detail.cbStatus = 3;
            }
    
            //memcpy(szBuffer + dwSendSize, &detail, sizeof(detail));
            //dwSendSize += sizeof(detail);
            vecWealthGodComingDetail.push_back(detail);
        }

        std::sort(vecWealthGodComingDetail.begin(), vecWealthGodComingDetail.end(), [](MSG_SC_WealthGodComing_Detail& a, MSG_SC_WealthGodComing_Detail& b)
        {
            return a.dwIndex > b.dwIndex;
        });

        for(auto& itVecDetail : vecWealthGodComingDetail)
        {
            memcpy(szBuffer + dwSendSize, &itVecDetail, sizeof(MSG_SC_WealthGodComing_Detail));
            dwSendSize += sizeof(MSG_SC_WealthGodComing_Detail);
        }
    }
    
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CActivityMgr::QueryRewardList(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_WealthGodComing_Reward_List))
    {
        return;
    }
    
    MSG_CS_WealthGodComing_Reward_List* pRewardList = (MSG_CS_WealthGodComing_Reward_List*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pRewardList->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }
    
    static CT_CHAR     szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD    dwSendSize = 0;
    static MSG_GameMsgDownHead msgHead;
    memset(&msgHead, 0, sizeof(msgHead));
    msgHead.dwMainID = MSG_WEALTHGODCOMING_MAIN;
    msgHead.dwSubID = SUB_SC_WEALTHGODCOMING_REWARD_LIST;
    msgHead.dwValue2 = pRewardList->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);
    
    static MSG_SC_WealthGodComing_RewardList rewardList;
    CT_DWORD  dwGameIndex = pRewardList->wGameID*100 + pRewardList->wKindID*10 + pRewardList->wRoomKind;
    
    auto itHistoryDoing = m_mapHistoryWealthGodComingData.find(dwGameIndex);
    if (itHistoryDoing != m_mapHistoryWealthGodComingData.end())
    {
        std::map<CT_DWORD, tagWealthGodComingData>& mapHistoryWealthGodComingData = itHistoryDoing->second;
        auto itHistoryData = mapHistoryWealthGodComingData.find(pRewardList->dwIndex);
        if (itHistoryData != mapHistoryWealthGodComingData.end())
        {
            tagWealthGodComingData& wealthGodComingData = itHistoryData->second;
            auto itCfg = m_mapWealthGodComingCfg.find(wealthGodComingData.wID);
            if (itCfg == m_mapWealthGodComingCfg.end())
            {
                LOG(WARNING) << "QueryRewardList can not wealth god coming cfg. activity id: " << wealthGodComingData.wID;
                return;
            }
            auto itHistoryGameUserInfo = m_mapHistoryWealthGodComingUserInfo.find(dwGameIndex);
            if (itHistoryGameUserInfo != m_mapHistoryWealthGodComingUserInfo.end())
            {
                std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo>>& mapHistoryUserInfo = itHistoryGameUserInfo->second;
                auto itMapHistoryUserInfo = mapHistoryUserInfo.find(pRewardList->dwIndex);
                if (itMapHistoryUserInfo != mapHistoryUserInfo.end())
                {
                    std::vector<tagWealthGodComingUserInfo>& vecUserInfo = itMapHistoryUserInfo->second;
                    if (!vecUserInfo.empty())
                    {
                        CT_WORD wMinUser = (vecUserInfo[0].cbRank != 255) ? 0 : itCfg->second.wSendRewardMinUser;
                        memcpy(szBuffer+dwSendSize, &wMinUser, sizeof(wMinUser));
                        dwSendSize += sizeof(wMinUser);

                        if (wMinUser == 0)
                        {
                            //首先找到自己的数据
                            for (auto& itVecHistoryUserInfo : vecUserInfo)
                            {
                                if (itVecHistoryUserInfo.dwUserID == pRewardList->dwUserID)
                                {
                                    memset(&rewardList, 0, sizeof(rewardList));
                                    rewardList.dwUserID = itVecHistoryUserInfo.dwUserID;
                                    rewardList.cbRank = itVecHistoryUserInfo.cbRank;
                                    rewardList.cbGender = itVecHistoryUserInfo.cbGender;
                                    rewardList.cbHeadID = itVecHistoryUserInfo.cbHeadIndex;
                                    rewardList.cbVip2 = itVecHistoryUserInfo.cbVip2;
                                    rewardList.dScore = itVecHistoryUserInfo.dwScore*TO_DOUBLE;
                                    rewardList.dReward = itVecHistoryUserInfo.dwRewardScore*TO_DOUBLE;
                                    rewardList.wValidCount = itVecHistoryUserInfo.wCompleteCount;
                                    rewardList.wDutyCompleteCount = itCfg->second.wCompleteCount;
                                    memcpy(szBuffer+dwSendSize, &rewardList, sizeof(rewardList));
                                    dwSendSize += sizeof(rewardList);
                                    break;
                                }
                            }

                            //查找其他人的数据
                            for (auto& itVecHistoryUserInfo : vecUserInfo)
                            {
                                if (itVecHistoryUserInfo.cbRank == 255)
                                    break;
                                memset(&rewardList, 0, sizeof(rewardList));
                                rewardList.dwUserID = itVecHistoryUserInfo.dwUserID;
                                rewardList.cbRank = itVecHistoryUserInfo.cbRank;
                                rewardList.cbGender = itVecHistoryUserInfo.cbGender;
                                rewardList.cbHeadID = itVecHistoryUserInfo.cbHeadIndex;
                                rewardList.cbVip2 = itVecHistoryUserInfo.cbVip2;
                                rewardList.dScore = itVecHistoryUserInfo.dwScore*TO_DOUBLE;
                                rewardList.dReward = itVecHistoryUserInfo.dwRewardScore*TO_DOUBLE;
                                rewardList.wValidCount = itVecHistoryUserInfo.wCompleteCount;
                                rewardList.wDutyCompleteCount = itCfg->second.wCompleteCount;
                                memcpy(szBuffer+dwSendSize, &rewardList, sizeof(rewardList));
                                dwSendSize += sizeof(rewardList);
                            }
                        }
                    }
                    else
                    {
                        CT_WORD wMinUser = itCfg->second.wSendRewardMinUser;
                        memcpy(szBuffer+dwSendSize, &wMinUser, sizeof(wMinUser));
                        dwSendSize += sizeof(wMinUser);
                    }
                }
                else
                {
                    CT_WORD wMinUser = itCfg->second.wSendRewardMinUser;
                    memcpy(szBuffer+dwSendSize, &wMinUser, sizeof(wMinUser));
                    dwSendSize += sizeof(wMinUser);
                }
            }
            else
            {
                CT_WORD wMinUser = itCfg->second.wSendRewardMinUser;
                memcpy(szBuffer+dwSendSize, &wMinUser, sizeof(wMinUser));
                dwSendSize += sizeof(wMinUser);
            }
        }
       
    }
    
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CActivityMgr::UpdateActivityIndexTodb(tagWealthGodComingLastIndex* pLastIndex)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_WGC_INDEX, pLastIndex, sizeof(tagWealthGodComingLastIndex));
}

void CActivityMgr::InsertActivityData(MSG_CS2DB_Insert_WGC_Data* pData)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_WGC_DATA, pData, sizeof(MSG_CS2DB_Insert_WGC_Data));
}

void CActivityMgr::UpdateActivetyData(MSG_CS2DB_Update_WGC_Data* pData)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_WGC_DATA, pData, sizeof(MSG_CS2DB_Update_WGC_Data));
}

void CActivityMgr::UpdateActivityUserInfo(MSG_CS2DB_Update_WGC_UserInfo* pUserInfo)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_WGC_USERINFO, pUserInfo, sizeof(MSG_CS2DB_Update_WGC_UserInfo));
}

void CActivityMgr::UpdateActivityAllUserRankInfo(CT_BYTE* pData, CT_DWORD dwSize)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_WGC_ALLUSERINFO, pData, dwSize);
}

void CActivityMgr::SendRewardByMail(CT_BYTE* pData, CT_DWORD dwSize)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, pData, dwSize);
}

bool CActivityMgr::IsSpecialTime(tm &timeNow, int nStartHour, int nStartMin, int nEndHour, int nEndMin)
{
    /*
    [8:20 <= x:yz < 10:40)
    <==> 8.20 <= x.yz < 10.40
    <==> 820 <= 100x + 10y + z  < 1040
    <==> 820 <= 100 * hour + minute  < 1040
    */
    int nStartTime = nStartHour* 100 + nStartMin;
    int nEndTime = nEndHour * 100 + nEndMin;
    
    int result = 100 * timeNow.tm_hour + timeNow.tm_min;
    if(result >= nStartTime && result < nEndTime)
        return true;
    
    return false;
}

void CActivityMgr::SendDoingWealthGodComingToClient(CT_DWORD dwUserID)
{
    static CT_BYTE  szBuffer[SYS_NET_SENDBUF_CLIENT];
    static CT_DWORD dwSendSize = 0;
    static MSG_GameMsgDownHead msgHead;
    if (m_bDoingActivityStatusChange)
    {
        dwSendSize = 0;
        memset(&msgHead, 0, sizeof(msgHead));
        msgHead.dwMainID = MSG_WEALTHGODCOMING_MAIN;
        msgHead.dwSubID = SUB_SC_CURRENT_WEALTHGODCOMING_ACTIVITY;
        dwSendSize += sizeof(MSG_GameMsgDownHead);

        MSG_SC_Current_WealthGodComing currentWealthGodComing;
        for (auto& it : m_mapDoingWealthGodComingData)
        {
            auto itCfg = m_mapWealthGodComingCfg.find(it.second.wID);
            if (itCfg != m_mapWealthGodComingCfg.end())
            {
                currentWealthGodComing.wGameID = itCfg->second.wGameID;
                currentWealthGodComing.wKindID = itCfg->second.wKindID;
                currentWealthGodComing.wRoomKindID = itCfg->second.wRoomKindID;
                memcpy(szBuffer + dwSendSize, &currentWealthGodComing, sizeof(currentWealthGodComing));
                dwSendSize += sizeof(currentWealthGodComing);
            }
        }
        m_bDoingActivityStatusChange = false;
    }

    if (dwUserID == 0)
    {
        MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto& it : mapAllGlobalUser)
        {
            acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
            if (pSocket==NULL)
            {
                continue;
            }
    
            msgHead.dwValue2 = it.first;
            memcpy(szBuffer, &msgHead, sizeof(msgHead));
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
        }
        
    }
    else
    {
        acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
        if (pSocket==NULL)
        {
            return;
        }
    
        msgHead.dwValue2 = dwUserID;
        memcpy(szBuffer, &msgHead, sizeof(msgHead));
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
    }
}

void CActivityMgr::BroadcastRealTimeRankInfo(CT_DWORD dwUserID/* = 0*/)
{
    if (m_mapDoingWealthGodComingData.empty())
        return;
    
    static std::map<CT_DWORD, MSG_SC_WealthGodComing_RealTimeInfo> mapRealTimeInfo;
    static std::map<CT_DWORD, std::map<CT_DWORD, CT_DWORD>> mapGameUserRank;
    
    if (dwUserID == 0)
    {
        mapGameUserRank.clear();
        mapRealTimeInfo.clear();
        
        //对玩家进行排名
        for (auto& it : m_mapDoingWealthGodComingUserInfo2)
        {
            std::vector<tagWealthGodComingUserInfo *> &vecUserInfo = it.second;
            std::sort(vecUserInfo.begin(),
                      vecUserInfo.end(),
                      [](tagWealthGodComingUserInfo *a, tagWealthGodComingUserInfo *b) {
                        return a->dwScore > b->dwScore;
                      });
        
            std::map<CT_DWORD, CT_DWORD> &mapUserRank = mapGameUserRank[it.first];
            CT_DWORD dwRank = 0;
            for (auto &itUserVec : vecUserInfo)
            {
                mapUserRank[itUserVec->dwUserID] = ++dwRank;
            }
        }
    
        tm timeNow;
        memset(&timeNow, 0, sizeof(timeNow));
        getLocalTime(&timeNow, time(NULL));
    
        MSG_SC_WealthGodComing_RealTimeInfo realTimeInfo;
        for (auto& itDoing : m_mapDoingWealthGodComingData)
        {
            auto itCfg = m_mapWealthGodComingCfg.find(itDoing.second.wID);
            if (itCfg != m_mapWealthGodComingCfg.end())
            {
                CT_DWORD dwTimeLeft = (itCfg->second.cbEndHour - timeNow.tm_hour)*60 + (itCfg->second.cbEndMin - timeNow.tm_min);
                CT_DWORD dwGameIndex = itCfg->second.wGameID*100+itCfg->second.wKindID*10+itCfg->second.wRoomKindID;
                realTimeInfo.dwMainID = MSG_WEALTHGODCOMING_MAIN;
                realTimeInfo.dwSubID = SUB_SC_WEALTHGODCOMING_REAL_TIME_INFO;
                realTimeInfo.dwTimeLeft = dwTimeLeft;
                realTimeInfo.wGameID = itCfg->second.wGameID;
                realTimeInfo.wKindID = itCfg->second.wKindID;
                realTimeInfo.wRoomKindID = itCfg->second.wRoomKindID;
                realTimeInfo.nTotalUser = mapGameUserRank[dwGameIndex].size();
                realTimeInfo.nRank = 0;
                mapRealTimeInfo[dwGameIndex] = realTimeInfo;
            }
        }
    }
    
    if (dwUserID == 0)
    {
        MapGlobalUser& mapGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto& it : mapGlobalUser)
        {
            tagGlobalUserInfo* pUserInfo = it.second->GetUserInfo();
            if (!pUserInfo || !pUserInfo->bOnline)
            {
                continue;
            }
        
            acl::aio_socket_stream* pClientSocket = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
            if (!pClientSocket)
            {
                continue;
            }
        
            CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(pUserInfo->dwGameServerID);
            if (!pGameServerInfo)
            {
                continue;
            }
        
            CT_DWORD  dwGameIndex = pGameServerInfo->wGameID*100+pGameServerInfo->wKindID*10+pGameServerInfo->wRoomKindID;
        
            auto itRealTimeInfo = mapRealTimeInfo.find(dwGameIndex);
            if (itRealTimeInfo == mapRealTimeInfo.end())
                continue;
        
            std::map<CT_DWORD, CT_DWORD>& mapUserRank = mapGameUserRank[dwGameIndex];
            auto itUserRank = mapUserRank.find(pUserInfo->dwUserID);
            if (itUserRank != mapUserRank.end())
            {
                itRealTimeInfo->second.nRank = itUserRank->second;
                //LOG(WARNING) << "set user rank, user id: "  << pUserInfo->dwUserID << ", user rank: " << itRealTimeInfo->second.nRank;
            }
            else
            {
                itRealTimeInfo->second.nRank = 0;
            }
            itRealTimeInfo->second.dwValue2 = pUserInfo->dwUserID;
            CNetModule::getSingleton().Send(pClientSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &itRealTimeInfo->second, sizeof(MSG_SC_WealthGodComing_RealTimeInfo));
            //LOG(WARNING) << "update user rank, user id: "  << pUserInfo->dwUserID << ", time left: " << itRealTimeInfo->second.dwTimeLeft;
        }
    }
    else
    {
        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
        if (!pUserInfo)
            return;
    
    
        acl::aio_socket_stream* pClientSocket = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (!pClientSocket)
        {
            return;
        }
    
        CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(pUserInfo->dwGameServerID);
        if (!pGameServerInfo)
        {
            return;
        }
    
        CT_DWORD  dwGameIndex = pGameServerInfo->wGameID*100+pGameServerInfo->wKindID*10+pGameServerInfo->wRoomKindID;
    
        auto itRealTimeInfo = mapRealTimeInfo.find(dwGameIndex);
        if (itRealTimeInfo == mapRealTimeInfo.end())
            return;
    
        std::map<CT_DWORD, CT_DWORD>& mapUserRank = mapGameUserRank[dwGameIndex];
        auto itUserRank = mapUserRank.find(pUserInfo->dwUserID);
        if (itUserRank != mapUserRank.end())
        {
            itRealTimeInfo->second.nRank = itUserRank->second;
        }
        else
        {
            itRealTimeInfo->second.nRank = 0;
        }
        itRealTimeInfo->second.dwValue2 = pUserInfo->dwUserID;
        CNetModule::getSingleton().Send(pClientSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &itRealTimeInfo->second, sizeof(MSG_SC_WealthGodComing_RealTimeInfo));
    }
}