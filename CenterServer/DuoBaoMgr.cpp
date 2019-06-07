//
// Created by luwei on 2018/10/20.
//
#include "stdafx.h"
#include "DuoBaoMgr.h"
#include <timeFunction.h>
#include "NetModule.h"
#include "ServerMgr.h"
#include "SystemMsgMgr.h"
#include <Utility.h>
#include <math.h>

extern CNetConnector *pNetDB;


CYYDuoBaoMgr::CYYDuoBaoMgr()
:m_bLoadData(false)
,m_cbSysState(0)
{

}

CYYDuoBaoMgr::~CYYDuoBaoMgr()
{

}

void CYYDuoBaoMgr::InsertDuoBaoCfg(tagDuoBaoCfg *pDuoBaoCfg)
{
    //加载配置重复
    auto it = m_mapDuoBaoCfg.find(pDuoBaoCfg->wID);
    if (it != m_mapDuoBaoCfg.end())
    {
        LOG(WARNING) << "insert duo bao cfg fail. repeated cfg! id: " << pDuoBaoCfg->wID;
        return;
    }
    
    //配置1
    m_mapDuoBaoCfg[pDuoBaoCfg->wID] = *pDuoBaoCfg;

    //配置2
    m_vecDuoBaoCfg.push_back(&m_mapDuoBaoCfg[pDuoBaoCfg->wID]);
    std::sort(m_vecDuoBaoCfg.begin(), m_vecDuoBaoCfg.end(), [](tagDuoBaoCfg* a, tagDuoBaoCfg* b)
    {
        return a->dwTotalCount < b->dwTotalCount;
    });
    //LOG(INFO) << "load duo bao cfg: " << pDuoBaoCfg->szTitle;
}

void CYYDuoBaoMgr::UpdateDuoBaoCfg(tagDuoBaoCfg* pDuoBaoCfg)
{
    auto it = m_mapDuoBaoCfg.find(pDuoBaoCfg->wID);
    if (it == m_mapDuoBaoCfg.end())
    {
        //如果是新插入数据
        LOG(WARNING) << "insert InsertDuoBaoCfg, id: " << pDuoBaoCfg->wID;
        InsertDuoBaoCfg(pDuoBaoCfg);
    }
    else
    {
        //更新数据
        it->second.wID = pDuoBaoCfg->wID;
        it->second.wSingleScore = pDuoBaoCfg->wSingleScore;
        it->second.dwUserLeaseScore = pDuoBaoCfg->dwUserLeaseScore;
        it->second.wlimitCount = pDuoBaoCfg->wlimitCount;
        it->second.dwTotalCount = pDuoBaoCfg->dwTotalCount;
        it->second.dwTimeLeft = pDuoBaoCfg->dwTimeLeft;
        it->second.dwReward = pDuoBaoCfg->dwReward;
        it->second.wRevenueRation = pDuoBaoCfg->wRevenueRation;
        //it->second.dwIndex = pDuoBaoCfg->dwIndex;这个值不能更新
        it->second.cbState = pDuoBaoCfg->cbState;
        _snprintf_info(it->second.szTitle, sizeof(it->second.szTitle), "%s", pDuoBaoCfg->szTitle);

        if (it->second.cbState == 0)
        {
            UpdateAllDuoBaoState();
        }

        //重新排序
        std::sort(m_vecDuoBaoCfg.begin(), m_vecDuoBaoCfg.end(), [](tagDuoBaoCfg* a, tagDuoBaoCfg* b)
        {
            return a->dwTotalCount < b->dwTotalCount;
        });

        LOG(WARNING) << "update InsertDuoBaoCfg, id: " << pDuoBaoCfg->wID;
    }
}

void CYYDuoBaoMgr::InsertDoingDuoBaoData(tagDuoBaoData* pDuoBaoData)
{
    auto it = m_mapDoingData.find(pDuoBaoData->wID);
    if (it != m_mapDoingData.end())
    {
        LOG(WARNING) << "has the same doing data? id: " << pDuoBaoData->wID;
        return;
    }

    std::unique_ptr<tagDuoBaoData> duobaoDataPtr(new tagDuoBaoData);
    duobaoDataPtr->wID = pDuoBaoData->wID;
    duobaoDataPtr->dwIndex = pDuoBaoData->dwIndex;
    duobaoDataPtr->dwLuckyUserID = pDuoBaoData->dwLuckyUserID;
    duobaoDataPtr->dwLuckyNum = pDuoBaoData->dwLuckyNum;
    duobaoDataPtr->dwRewardScore = pDuoBaoData->dwRewardScore;
    duobaoDataPtr->dwStartTime = pDuoBaoData->dwStartTime;
    duobaoDataPtr->dwEndTime = pDuoBaoData->dwEndTime;
    duobaoDataPtr->cbState = pDuoBaoData->cbState;

    m_mapDoingData.insert(std::make_pair(pDuoBaoData->wID, std::move(duobaoDataPtr)));
    //LOG(WARNING) << "load doing duo bao data, id: " << pDuoBaoData->wID;
}

void CYYDuoBaoMgr::InsertDoingDuoBaoUser(tagDuoBaoUserInfo* pDuoBaoUser)
{
    CT_DWORD dwIndex = pDuoBaoUser->wID*DUOBAO_INDEX + pDuoBaoUser->dwIndex;
    MapDuoBaoUserDetail& mapUser = m_mapDoingUser[dwIndex];

    std::unique_ptr<tagDuoBaoUserInfo> userInfoPtr(new tagDuoBaoUserInfo);
    userInfoPtr->wID = pDuoBaoUser->wID;
    userInfoPtr->dwIndex = pDuoBaoUser->dwIndex;
    userInfoPtr->dwUserID = pDuoBaoUser->dwUserID;
    userInfoPtr->cbSex = pDuoBaoUser->cbSex;
    userInfoPtr->cbHeadIndex = pDuoBaoUser->cbHeadIndex;
    userInfoPtr->dwLuckyNum = pDuoBaoUser->dwLuckyNum;

    VecDuoBaoUserInfo& vecUserInfo = mapUser[pDuoBaoUser->dwUserID];
    vecUserInfo.push_back(std::move(userInfoPtr));

    //当期幸运码
    MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];
    mapLuckyNum.insert(std::make_pair(pDuoBaoUser->dwLuckyNum, pDuoBaoUser->dwUserID));

    //LOG(WARNING) << "load doing user: " << pDuoBaoUser->dwUserID << ", lucky num: " << pDuoBaoUser->dwLuckyNum;
}

void CYYDuoBaoMgr::InsertHistoryDuoBaoData(tagDuoBaoData* pDuoBaoData)
{
    ListHistoryDuoBaoData& listHistoryDuoBaoData = m_mapHistoryData[pDuoBaoData->wID];

    std::unique_ptr<tagDuoBaoData> duobaoDataPtr(new tagDuoBaoData);
    duobaoDataPtr->wID = pDuoBaoData->wID;
    duobaoDataPtr->dwIndex = pDuoBaoData->dwIndex;
    duobaoDataPtr->dwLuckyUserID = pDuoBaoData->dwLuckyUserID;
    duobaoDataPtr->dwLuckyNum = pDuoBaoData->dwLuckyNum;
    duobaoDataPtr->dwRewardScore = pDuoBaoData->dwRewardScore;
    duobaoDataPtr->dwStartTime = pDuoBaoData->dwStartTime;
    duobaoDataPtr->dwEndTime = pDuoBaoData->dwEndTime;
    duobaoDataPtr->cbState = pDuoBaoData->cbState;

    //LOG(WARNING) << "load duo bao history, id: " << duobaoDataPtr->wID << ", index: " << duobaoDataPtr->dwIndex;
    listHistoryDuoBaoData.push_back(std::move(duobaoDataPtr));
}

void CYYDuoBaoMgr::InsertHistoryDuoBaoUser(tagDuoBaoUserInfo* pDuoBaoUser)
{
    CT_DWORD dwIndex = pDuoBaoUser->wID*DUOBAO_INDEX + pDuoBaoUser->dwIndex;
    MapDuoBaoUserDetail& mapUser = m_mapHistoryUser[dwIndex];

    std::unique_ptr<tagDuoBaoUserInfo> userInfoPtr(new tagDuoBaoUserInfo);
    userInfoPtr->wID = pDuoBaoUser->wID;
    userInfoPtr->dwIndex = pDuoBaoUser->dwIndex;
    userInfoPtr->dwUserID = pDuoBaoUser->dwUserID;
    userInfoPtr->cbSex = pDuoBaoUser->cbSex;
    userInfoPtr->cbHeadIndex = pDuoBaoUser->cbHeadIndex;
    userInfoPtr->dwLuckyNum = pDuoBaoUser->dwLuckyNum;

    VecDuoBaoUserInfo& vecUserInfo = mapUser[pDuoBaoUser->dwUserID];
    //LOG(WARNING) << "load duo bao user history, id: " << userInfoPtr->wID << ", index: " << userInfoPtr->dwIndex << ", user id: " <<  userInfoPtr->dwUserID;
    vecUserInfo.push_back(std::move(userInfoPtr));
}

void CYYDuoBaoMgr::SetLoadDataFinish(bool bLoadData)
{
    m_bLoadData = bLoadData;
    m_cbSysState = 1;
    LOG(WARNING) << "load duo bao data finish.";
}

void CYYDuoBaoMgr::SetSystemState(CT_BYTE cbState)
{
    m_cbSysState = cbState;
}

void CYYDuoBaoMgr::CheckDuoBaoStartEnd()
{
    if (!m_bLoadData)
        return;

    CT_DWORD dwNow = (CT_DWORD)time(NULL);
    //检测结束
    for (auto it = m_mapDoingData.begin(); it != m_mapDoingData.end(); )
    {
        std::unique_ptr<tagDuoBaoData>& duobaoDataPtr = it->second;
        CT_DWORD& dwStartTime = duobaoDataPtr->dwStartTime;
        CT_DWORD& dwEndTime = duobaoDataPtr->dwEndTime;

        bool bErase = false;

        //检测超过一天的活动
        if (duobaoDataPtr->cbState == 0)
        {
            CT_DWORD dwInterval = dwNow - dwStartTime;
            auto itCfg = m_mapDuoBaoCfg.find(it->first);
            if (itCfg != m_mapDuoBaoCfg.end() && dwInterval >= itCfg->second.dwTimeLeft)
            {
                //duobaoDataPtr->cbState = 1;
                //dwEndTime = dwNow;
                //UpdateDoingDuoBaoState(duobaoDataPtr, 0);
                //流局
                FlowOneDuoBao(duobaoDataPtr);
                UpdateDataToDB(duobaoDataPtr);
                UpdateDoingDuoBaoState(duobaoDataPtr, 0);
            }
        }

        //检测开奖状态的活动
        if (duobaoDataPtr->cbState == 1)
        {
            CT_DWORD dwInterval = dwNow - dwEndTime;
            if (dwInterval >= 4)
            {
                auto itCfg = m_mapDuoBaoCfg.find(it->second->wID);
                if (itCfg == m_mapDuoBaoCfg.end())
                {
                    LOG(WARNING) << "can not find duobao cfg. id: " << it->second->wID;
                    //it = m_mapDoingData.erase(it);
                    duobaoDataPtr->cbState = 3;
                    ++it;
                    continue;
                }

                //检测时间是否已经达到足够的人数开奖
                CT_DWORD dwIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;
                MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];

                //如果不够人数开奖(实际不会出现这个情况，在上面的判断已经清理过期数据)
                if (mapLuckyNum.size() < itCfg->second.dwTotalCount)
                {
                    //流局
                    FlowOneDuoBao(duobaoDataPtr);
                }
                else
                {
                    //正式开奖
                    OpenOneDuoBao(duobaoDataPtr);
                    duobaoDataPtr->dwRewardScore = itCfg->second.dwReward*TO_LL*(1.0f - (ceil)(itCfg->second.wRevenueRation)/100);
                }
                UpdateDataToDB(duobaoDataPtr);
                UpdateDoingDuoBaoState(duobaoDataPtr, 0);
            }
        }
        else if (duobaoDataPtr->cbState == 2 )
        {
            CT_DWORD dwInterval = dwNow - dwEndTime;
            if (dwInterval >= 10)
            {
                auto itCfg = m_mapDuoBaoCfg.find(duobaoDataPtr->wID);
                if (itCfg != m_mapDuoBaoCfg.end())
                {
                    //发奖励
                    tagUserMail mail;
                    mail.cbState = 0;
                    mail.cbMailType = 2;
                    mail.dwSendTime = Utility::GetTime();
                    mail.dwExpiresTime = mail.dwSendTime + 89600 * 30; //默认一个月的有效期
                    mail.dwUserID = duobaoDataPtr->dwLuckyUserID;
                    mail.llScore = duobaoDataPtr->dwRewardScore;
                    mail.cbScoreChangeType = YY_DUOBAO_REWARD;
                    _snprintf_info(mail.szTitle, sizeof(mail.szTitle), "夺宝奖励");
                    _snprintf_info(mail.szContent, sizeof(mail.szContent), "恭喜您在夺宝活动'%s'第%d期中奖，奖励%0.2f金币!",
                        itCfg->second.szTitle, duobaoDataPtr->dwIndex, duobaoDataPtr->dwRewardScore*TO_DOUBLE);
                    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, &mail, sizeof(mail));
                    //发送邮件消息提醒
                    CUserMgr::get_instance().SendNewMailTip(mail.dwUserID);
                }

                //清理数据
                CT_DWORD dwIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;
                MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];

                ClearOpenDuoBao(duobaoDataPtr);
                mapLuckyNum.erase(dwIndex);

                it = m_mapDoingData.erase(it);
                bErase = true;
            }
        }
        else if (duobaoDataPtr->cbState == 3)
        {
            //清理数据
            CT_DWORD dwIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;
            MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];

            auto itCfg = m_mapDuoBaoCfg.find(duobaoDataPtr->wID);
            if (itCfg != m_mapDuoBaoCfg.end())
            {
                //统计每个购买了多少次
                std::map<CT_DWORD, CT_DWORD> mapBuyLuckyNum;
                for (auto& it : mapLuckyNum)
                {
                    auto itBuy = mapBuyLuckyNum.find(it.second);
                    if (itBuy != mapBuyLuckyNum.end())
                    {
                        ++itBuy->second;
                    }
                    else
                    {
                        mapBuyLuckyNum.insert(std::make_pair(it.second, 1));
                    }
                }

                tagUserMail mail;
                //返还购买的幸运码
                for (auto& it : mapBuyLuckyNum)
                {
                    mail.cbState = 0;
                    mail.cbMailType = 2;
                    mail.dwSendTime = Utility::GetTime();
                    mail.dwExpiresTime = mail.dwSendTime + 89600 * 30; //默认一个月的有效期
                    mail.dwUserID = it.first;
                    mail.llScore = itCfg->second.wSingleScore*TO_LL * it.second;
                    mail.cbScoreChangeType = YY_DUOBAO_RETURN;
                    _snprintf_info(mail.szTitle, sizeof(mail.szTitle), "夺宝返还");
                    _snprintf_info(mail.szContent, sizeof(mail.szContent), "因活动'%s'第%d期过期没有开奖，你总共购买%d个幸运码，每个幸运码价值%d金币, 合计返还你购买幸运码的%d金币",
                        itCfg->second.szTitle, duobaoDataPtr->dwIndex, it.second, itCfg->second.wSingleScore, itCfg->second.wSingleScore * it.second);
                    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, &mail, sizeof(mail));
                    //发送邮件消息提醒
                    CUserMgr::get_instance().SendNewMailTip(mail.dwUserID);
                }
            }

            ClearOpenDuoBao(duobaoDataPtr);
            mapLuckyNum.erase(dwIndex);

            it = m_mapDoingData.erase(it);
            bErase = true;
        }

        if (!bErase)
        {
            ++it;
        }
    }

    //检测开始
    for (auto& it : m_mapDuoBaoCfg)
    {
        //状态为0的标识为不能再开始新活动
        if (it.second.cbState == 0)
            continue;

        auto itDoing = m_mapDoingData.find(it.first);
        if (itDoing == m_mapDoingData.end())
        {
            //开始一场新活动
            it.second.dwIndex += 1;
            std::unique_ptr<tagDuoBaoData> duobaoDataPtr(new tagDuoBaoData);
            duobaoDataPtr->wID = it.second.wID;
            duobaoDataPtr->dwIndex = it.second.dwIndex;
            duobaoDataPtr->dwStartTime = dwNow;
            duobaoDataPtr->dwEndTime = 0;
            duobaoDataPtr->dwLuckyUserID = 0;
            duobaoDataPtr->cbState = 0;
            //插入数据库
            UpdateDataToDB(duobaoDataPtr);
            UpdateDoingDuoBaoState(duobaoDataPtr, 0);
            m_mapDoingData.insert(std::make_pair(duobaoDataPtr->wID, std::move(duobaoDataPtr)));
        }
    }

    /*static int testCount = 0;
    if (testCount < 2000)
    {
        auto itDoingEx = m_mapDoingData.begin();
        std::unique_ptr<tagDuoBaoData>& duobaoDataPtrEx = itDoingEx->second;
        if (duobaoDataPtrEx->cbState == 0)
        {
            MSG_CS_Buy_LuckyNum buyLuckyNum;
            buyLuckyNum.wID = itDoingEx->first;
            buyLuckyNum.dwBuyCount = 10;
            buyLuckyNum.dwIndex = duobaoDataPtrEx->dwIndex;
            buyLuckyNum.dwUserID = 85033036;
            BuyLuckyNum(&buyLuckyNum, sizeof(buyLuckyNum));
            ++testCount;
        }
    }*/
}

void CYYDuoBaoMgr::FlowOneDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr)
{
    //auto itLuckyNum = m_mapDoingLuckNum.find(duobaoDataPtr->wID);
    duobaoDataPtr->dwLuckyUserID = 0;
    duobaoDataPtr->cbState = 3;
    duobaoDataPtr->dwEndTime = (CT_DWORD)time(NULL);
}

void CYYDuoBaoMgr::OpenOneDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr)
{
    std::vector<CT_DWORD> vecLuckyNum;
    CT_DWORD dwDuoBaoIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;
    auto itDoingLucyNum = m_mapDoingLuckNum.find(dwDuoBaoIndex);
    if (itDoingLucyNum != m_mapDoingLuckNum.end())
    {
        MapLuckyNum& mapLuckyNum = itDoingLucyNum->second;
        for (auto& itLuckNum : mapLuckyNum)
        {
            vecLuckyNum.push_back(itLuckNum.first);
        }

        //随机一个号码中奖
        if (vecLuckyNum.empty())
            return;

        auto nLuckyIndex = rand() % vecLuckyNum.size();
        auto nLuckyNum = vecLuckyNum[nLuckyIndex];

        auto itUser = mapLuckyNum.find(nLuckyNum);
        if (itUser == mapLuckyNum.end())
        {
            LOG(WARNING) << "open lucky num error. id: " << duobaoDataPtr->wID << ", index: " << duobaoDataPtr->dwIndex;
            return;
        }

        duobaoDataPtr->dwLuckyUserID = itUser->second;
        duobaoDataPtr->dwLuckyNum = nLuckyNum;
        duobaoDataPtr->cbState = 2;
        duobaoDataPtr->dwEndTime = (CT_DWORD)time(NULL);
    }
}

void CYYDuoBaoMgr::ClearOpenDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr)
{
    CT_DWORD dwDuoBaoIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;

    //把记录放进历史记录里面
    ListHistoryDuoBaoData& listHistoryDuoBaoData =  m_mapHistoryData[duobaoDataPtr->wID];
    listHistoryDuoBaoData.push_back(std::move(duobaoDataPtr));
    //把用户放到历史记录里面
    MapDuoBaoUserDetail& mapDoingUserDetail = m_mapDoingUser[dwDuoBaoIndex];
    MapDuoBaoUserDetail& mapHistoryUserDetail = m_mapHistoryUser[dwDuoBaoIndex];
    for (auto& itDoingUser : mapDoingUserDetail)
    {
        VecDuoBaoUserInfo& vecHistoryUserInfo = mapHistoryUserDetail[itDoingUser.first];
        VecDuoBaoUserInfo& vecDoingUserInfo = itDoingUser.second;

        for (auto& itDoingUserInfo : vecDoingUserInfo)
        {
            vecHistoryUserInfo.push_back(std::move(itDoingUserInfo));
        }
    }
    //删除用户详细信息
    mapDoingUserDetail.clear();
    m_mapDoingUser.erase(dwDuoBaoIndex);

    //遍历链表，把10天前的数据删除
    /*CT_DWORD dwNow = time(NULL);
    for (auto itHistoryData = listHistoryDuoBaoData.begin(); itHistoryData != listHistoryDuoBaoData.end(); )
    {
        if (dwNow - (*itHistoryData)->dwEndTime > 10*DAY_SECOND)
        {
            //删除对应的用户资料
            CT_DWORD dwDuoBaoHistoryIndex = (*itHistoryData)->wID*DUOBAO_INDEX+(*itHistoryData)->dwIndex;
            auto itHistoryUser = m_mapHistoryUser.find(dwDuoBaoHistoryIndex);
            if (itHistoryUser != m_mapHistoryUser.end())
            {
                m_mapHistoryUser.erase(itHistoryUser);
            }

            //删除用户资料
            itHistoryData = listHistoryDuoBaoData.erase(itHistoryData);
        }
        else
        {
            ++itHistoryData;
        }
    }*/

    //清除20局以前的数据
    if (listHistoryDuoBaoData.size() > 20)
    {
        auto itHistoryData = listHistoryDuoBaoData.begin();
        //删除对应的用户资料
        CT_DWORD dwDuoBaoHistoryIndex = (*itHistoryData)->wID*DUOBAO_INDEX+(*itHistoryData)->dwIndex;
        auto itHistoryUser = m_mapHistoryUser.find(dwDuoBaoHistoryIndex);
        if (itHistoryUser != m_mapHistoryUser.end())
        {
            m_mapHistoryUser.erase(itHistoryUser);
        }

        //删除用户资料
        itHistoryData = listHistoryDuoBaoData.erase(itHistoryData);
    }
}

void CYYDuoBaoMgr::UpdateDoingDuoBaoState(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr, CT_DWORD dwUserID)
{
    if (m_setOpenUiUser.empty())
        return;


   /* CT_BYTE     cbState;                //状态(0新开一场活动 1表示正在开奖 2表示已经开奖 3表示过期不开奖)当状态为2时,附带幸运儿的ID和头像，幸运码
    CT_WORD     wID;                    //活动ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD 	dwLuckyUserID;			//幸运儿ID
    CT_DWORD 	dwLuckyNum;				//幸运码
    CT_BYTE 	cbSex;					//幸运儿性别
    CT_BYTE 	cbHeadIndex;			//幸运儿头像ID*/

    MSG_SC_Update_DuoBao_State duoBaoState;
    duoBaoState.dwMainID = MSG_DUOBAO_MAIN;
    duoBaoState.dwSubID = SUB_SC_UPDATE_DUOBAO_STATE;
    duoBaoState.wID = duobaoDataPtr->wID;
    duoBaoState.dwIndex = duobaoDataPtr->dwIndex;
    duoBaoState.cbState = duobaoDataPtr->cbState;

    if (duoBaoState.cbState == 2)
    {
        duoBaoState.dwLuckyUserID = duobaoDataPtr->dwLuckyUserID;
        duoBaoState.dwLuckyNum = duobaoDataPtr->dwLuckyNum;

        CT_DWORD dwIndex = duobaoDataPtr->wID*DUOBAO_INDEX+duobaoDataPtr->dwIndex;
        MapDuoBaoUserDetail& mapDuoBaoUserDetail = m_mapDoingUser[dwIndex];
        auto itLuckyUser = mapDuoBaoUserDetail.find(duobaoDataPtr->dwLuckyUserID);
        if (itLuckyUser != mapDuoBaoUserDetail.end())
        {
            VecDuoBaoUserInfo& vecDuoBaoUserInfo = itLuckyUser->second;
            if (!vecDuoBaoUserInfo.empty())
            {
                duoBaoState.cbSex = vecDuoBaoUserInfo[0]->cbSex;
                duoBaoState.cbHeadIndex = vecDuoBaoUserInfo[0]->cbHeadIndex;
            }
        }
    }

    if (dwUserID == 0)
    {
        for (auto itOpenUser = m_setOpenUiUser.begin(); itOpenUser != m_setOpenUiUser.end(); )
        {
            acl::aio_socket_stream* pOpenUiUserSocket = CUserMgr::get_instance().FindUserProxySocksPtr(*itOpenUser);
            if (pOpenUiUserSocket == NULL)
            {
                itOpenUser = m_setOpenUiUser.erase(itOpenUser);
                continue;
            }
            duoBaoState.dwValue2 = *itOpenUser;
            CNetModule::getSingleton().Send(pOpenUiUserSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &duoBaoState, sizeof(duoBaoState));
            ++itOpenUser;
        }
    }
    else
    {
        acl::aio_socket_stream* pOpenUiUserSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
        if (pOpenUiUserSocket != NULL)
        {
            duoBaoState.dwValue2 = dwUserID;
            CNetModule::getSingleton().Send(pOpenUiUserSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &duoBaoState, sizeof(duoBaoState));
        }
    }
}

void CYYDuoBaoMgr::UpdateAllDuoBaoState()
{
    for (auto itOpenUser = m_setOpenUiUser.begin(); itOpenUser != m_setOpenUiUser.end(); )
    {
        acl::aio_socket_stream* pOpenUiUserSocket = CUserMgr::get_instance().FindUserProxySocksPtr(*itOpenUser);
        if (pOpenUiUserSocket == NULL)
        {
            itOpenUser = m_setOpenUiUser.erase(itOpenUser);
            continue;
        }

        MSG_CS_DuoBao_Main mainInfo;
        mainInfo.dwUserID = *itOpenUser;
        QueryMainInfo(&mainInfo, sizeof(mainInfo));
        ++itOpenUser;
    }
}

void CYYDuoBaoMgr::DuoBaoMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
 {
    switch(wSubCmdID)
    {
        case SUB_CS_DUOBAO_MAIN_INFO:
        {
            QueryMainInfo(pData, wDataSize);
            break;
        }

        case SUB_CS_QUERY_SELF_LUCKY_NUM:
        {
            QuerySelfLuckyNum(pData, wDataSize);
            break;
        }

        case SUB_CS_QUERY_SELF_HISTORY:
        {
            QuerySelfHistory(pData, wDataSize);
            break;
        }

        case SUB_CS_QUERY_HISTORY_LUCKY_USER:
        {
            QueryHistoryLuckyUser(pData, wDataSize);
            break;
        }
        case SUB_CS_BUY_LUCKY_NUM:
        {
            BuyLuckyNum(pData, wDataSize);
        }
            break;
        case SUB_CS_CLOSE_UI:
        {
            CloseUi(pData, wDataSize);
            break;
        }
        case SUB_CS_QUERY_REWARD_RECORD:
        {
            QueryRewardRecord(pData, wDataSize);
            break;
        }
        default:break;
    }
}

void CYYDuoBaoMgr::QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_DuoBao_Main))
    {
        return;
    }

    MSG_CS_DuoBao_Main* pQueryMain = (MSG_CS_DuoBao_Main*)pData;
    //acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryMain->dwUserID);
    //if (pSocket == NULL)
    //{
    //    return;
    //}

    //玩家数据
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryMain->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    if (!pUserInfo->bOnline)
    {
        return;
    }

    acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pSocket == NULL)
    {
        return;
    }


    static CT_CHAR      szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD            dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_DUOBAO_MAIN;
    msgHead.dwSubID = SUB_SC_DUOBAO_MAIN_INFO;
    msgHead.dwValue2 = pQueryMain->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_DuoBao_Main mainInfo;
    std::vector<CT_WORD > vecEndDuoBao;
    for (auto itCfg : m_vecDuoBaoCfg)
    {
        auto itDoing = m_mapDoingData.find(itCfg->wID);
        if (itDoing == m_mapDoingData.end())
        {
            continue;
        }

        if (itDoing->second->cbState == 2)
        {
            vecEndDuoBao.push_back(itCfg->wID);
        }

        mainInfo.wID = itCfg->wID;
        mainInfo.dwIndex = itCfg->dwIndex;
        mainInfo.dwUnitPrice = itCfg->wSingleScore;
        mainInfo.dwUserLeaseScore = itCfg->dwUserLeaseScore;
        mainInfo.dwTotalCount = itCfg->dwTotalCount;
        mainInfo.dwRewardScore = itCfg->dwReward;
        mainInfo.wlimitCount = itCfg->wlimitCount;
        _snprintf_info(mainInfo.szTitle, sizeof(mainInfo.szTitle), "%s", itCfg->szTitle);

        CT_DWORD dwIndex = itCfg->wID*DUOBAO_INDEX+itCfg->dwIndex;
        MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];
        mainInfo.dwBuyCount = (CT_DWORD)mapLuckyNum.size();
        MapDuoBaoUserDetail& mapDuoBaoUserDetail = m_mapDoingUser[dwIndex];
        auto itDoingUser = mapDuoBaoUserDetail.find(pQueryMain->dwUserID);
        if (itDoingUser == mapDuoBaoUserDetail.end())
        {
            mainInfo.dwSelfBuyCount = 0;
        }
        else
        {
            VecDuoBaoUserInfo& vecDuoBaoUserInfo = itDoingUser->second;
            mainInfo.dwSelfBuyCount = (CT_DWORD)vecDuoBaoUserInfo.size();
        }
        mainInfo.cbState = itDoing->second->cbState;
        memcpy(szBuffer+dwSendSize, &mainInfo, sizeof(MSG_SC_DuoBao_Main));
        dwSendSize += sizeof(MSG_SC_DuoBao_Main);
    }

    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);

    //插入打开UI列表
    auto itOpenUiUser = m_setOpenUiUser.find(pQueryMain->dwUserID);
    if (itOpenUiUser == m_setOpenUiUser.end())
    {
        m_setOpenUiUser.insert(pQueryMain->dwUserID);
        //如果玩家不在游戏房间
        if (pUserInfo->dwGameServerID == 0)
        {
            pUserInfo->dwGameServerID = 1000;
        }
    }

    //如果有结束的活动，把活动的数据发送给客户端
    if (!vecEndDuoBao.empty())
    {
        for (auto& it :vecEndDuoBao)
        {
            auto itDoing = m_mapDoingData.find(it);
            if (itDoing == m_mapDoingData.end())
            {
                continue;
            }
            UpdateDoingDuoBaoState(itDoing->second, pQueryMain->dwUserID);
        }
    }
}

void CYYDuoBaoMgr::QuerySelfLuckyNum(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Self_DuoBao_LuckyNum))
    {
        return;
    }

    MSG_CS_Query_Self_DuoBao_LuckyNum* pQueryLuckyNum = (MSG_CS_Query_Self_DuoBao_LuckyNum*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryLuckyNum->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    static CT_CHAR      szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD            dwSendSize = 0;
    CT_DWORD            dwLuckyNumCount = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_DUOBAO_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_SELF_LUCKY_NUM;
    msgHead.dwValue2 = pQueryLuckyNum->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Query_Self_DuoBao_LuckyNum luckyNum;
    luckyNum.wID = pQueryLuckyNum->wID;
    luckyNum.dwIndex = pQueryLuckyNum->dwIndex;
    memcpy(szBuffer+dwSendSize, &luckyNum, sizeof(MSG_SC_Query_Self_DuoBao_LuckyNum));
    dwSendSize += sizeof(MSG_SC_Query_Self_DuoBao_LuckyNum);

    CT_DWORD dwIndex = pQueryLuckyNum->wID*DUOBAO_INDEX+pQueryLuckyNum->dwIndex;
    auto itDoingUser = m_mapDoingUser.find(dwIndex);
    if (itDoingUser != m_mapDoingUser.end())
    {
        MapDuoBaoUserDetail& mapDuoBaoUserDetail = itDoingUser->second;
        auto itDoingUserDetail = mapDuoBaoUserDetail.find(pQueryLuckyNum->dwUserID);
        if (itDoingUserDetail != mapDuoBaoUserDetail.end())
        {
            VecDuoBaoUserInfo& vecDuoBaoUserInfo = itDoingUserDetail->second;
            for (auto& itvecUser : vecDuoBaoUserInfo)
            {
                memcpy(szBuffer+dwSendSize, &itvecUser->dwLuckyNum, sizeof(CT_DWORD));
                dwSendSize += sizeof(CT_DWORD);
                if (++dwLuckyNumCount >= 200)
                {
                    break;
                }
            }
        }
    }
    else
    {
        auto itHistoryUser = m_mapHistoryUser.find(dwIndex);
        if (itHistoryUser != m_mapHistoryUser.end())
        {
            MapDuoBaoUserDetail& mapDuoBaoUserDetail = itHistoryUser->second;
            auto itDoingUserDetail = mapDuoBaoUserDetail.find(pQueryLuckyNum->dwUserID);
            if (itDoingUserDetail != mapDuoBaoUserDetail.end())
            {
                VecDuoBaoUserInfo& vecDuoBaoUserInfo = itDoingUserDetail->second;
                for (auto& itvecUser : vecDuoBaoUserInfo)
                {
                    if (itvecUser.get() == nullptr)
                    {
                        LOG(WARNING) << "history duo bao user nullptr. userid : " << pQueryLuckyNum->dwUserID << ", index: " << pQueryLuckyNum->dwIndex;
                        continue;
                    }
                    memcpy(szBuffer+dwSendSize, &itvecUser->dwLuckyNum, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                    if (++dwLuckyNumCount >= 200)
                    {
                        break;
                    }
                }
            }
        }
    }
    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CYYDuoBaoMgr::QuerySelfHistory(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Self_DuoBao_History))
    {
        return;
    }

    MSG_CS_Query_Self_DuoBao_History* pQueryHistory = (MSG_CS_Query_Self_DuoBao_History*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryHistory->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    static CT_CHAR      szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD            dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_DUOBAO_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_SELF_HISTORY;
    msgHead.dwValue2 = pQueryHistory->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Query_Self_DuoBao_History selfDuoBaoHistory;

    /*CT_WORD     wID;                    //夺宝ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD 	dwBuyCount;			    //已经购买次数
    CT_DWORD 	dwSelfBuyCount; 	    //自己购买次数
    CT_DWORD    dwTotalCount;           //总次数
    CT_DWORD    dwEndTime;              //结束时间戳
    CT_DWORD    dwLuckyNum;             //幸运码
    CT_DWORD    dwLuckyUser;            //幸运儿
    CT_BYTE     cbLuckySex;             //幸运儿性别
    CT_BYTE     cbLuckyHeadIndex;       //幸运儿头像索引
    CT_CHAR 	szTitle[DUOBAO_TITLE];  //夺宝标题
    CT_BYTE     cbState;                //状态(0表示没有开奖 1表示正在开奖 2表示已经开奖 3表示过期不开奖)*/

    for (auto& itDoing : m_mapDoingData)
    {
        auto itCfg = m_mapDuoBaoCfg.find(itDoing.first);
        if (itCfg == m_mapDuoBaoCfg.end())
            continue;

        std::unique_ptr<tagDuoBaoData>& doingDataPtr = itDoing.second;

        CT_DWORD dwIndex = doingDataPtr->wID*DUOBAO_INDEX + doingDataPtr->dwIndex;
        MapDuoBaoUserDetail& mapDuoBaoUserDetail = m_mapDoingUser[dwIndex];
        CT_DWORD dwSelfBuyCount = 0;
       /* if (!mapDuoBaoUserDetail.empty())
        {

        }*/
        auto itDoingUser = mapDuoBaoUserDetail.find(pQueryHistory->dwUserID);
        if (itDoingUser == mapDuoBaoUserDetail.end())
        {
            continue;
        }
        dwSelfBuyCount = itDoingUser->second.size();
        MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];

        memset(&selfDuoBaoHistory, 0, sizeof(selfDuoBaoHistory));
        selfDuoBaoHistory.wID = doingDataPtr->wID;
        selfDuoBaoHistory.dwIndex = doingDataPtr->dwIndex;
        selfDuoBaoHistory.dwBuyCount = mapLuckyNum.size();
        selfDuoBaoHistory.dwSelfBuyCount = dwSelfBuyCount;
        selfDuoBaoHistory.dwTotalCount = itCfg->second.dwTotalCount;
        selfDuoBaoHistory.dwEndTime = doingDataPtr->dwEndTime;
        selfDuoBaoHistory.dwLuckyNum = doingDataPtr->dwLuckyNum;
        selfDuoBaoHistory.dwLuckyUser = doingDataPtr->dwLuckyUserID;
        selfDuoBaoHistory.cbState = doingDataPtr->cbState;
        _snprintf_info(selfDuoBaoHistory.szTitle, sizeof(selfDuoBaoHistory.szTitle), "%s", itCfg->second.szTitle);

        memcpy(szBuffer+dwSendSize, &selfDuoBaoHistory, sizeof(selfDuoBaoHistory));
        dwSendSize += sizeof(selfDuoBaoHistory);
    }

    for (auto& itHistory : m_mapHistoryData)
    {
        auto itCfg = m_mapDuoBaoCfg.find(itHistory.first);
        if (itCfg == m_mapDuoBaoCfg.end())
            continue;

        ListHistoryDuoBaoData& listHistoryDuoBaoData = itHistory.second;
        for (auto itListHistory = listHistoryDuoBaoData.rbegin(); itListHistory != listHistoryDuoBaoData.rend(); ++itListHistory)
        {
            CT_DWORD dwIndex = (*itListHistory)->wID*DUOBAO_INDEX + (*itListHistory)->dwIndex;
            auto itHistoryUser = m_mapHistoryUser.find(dwIndex);
            if (itHistoryUser == m_mapHistoryUser.end())
            {
                continue;
            }

            MapDuoBaoUserDetail& mapDuoBaoUserDetail = itHistoryUser->second;
            auto itUserDetail = mapDuoBaoUserDetail.find(pQueryHistory->dwUserID);
            if (itUserDetail == mapDuoBaoUserDetail.end())
            {
                continue;
            }

            memset(&selfDuoBaoHistory, 0, sizeof(selfDuoBaoHistory));
            selfDuoBaoHistory.wID = (*itListHistory)->wID;
            selfDuoBaoHistory.dwIndex = (*itListHistory)->dwIndex;
            selfDuoBaoHistory.dwBuyCount = itCfg->second.dwTotalCount;
            selfDuoBaoHistory.dwSelfBuyCount = (*itUserDetail).second.size();
            selfDuoBaoHistory.dwTotalCount = itCfg->second.dwTotalCount;
            selfDuoBaoHistory.dwEndTime = (*itListHistory)->dwEndTime;
            selfDuoBaoHistory.dwLuckyNum = (*itListHistory)->dwLuckyNum;
            selfDuoBaoHistory.dwLuckyUser = (*itListHistory)->dwLuckyUserID;
            selfDuoBaoHistory.cbState = (*itListHistory)->cbState;
            _snprintf_info(selfDuoBaoHistory.szTitle, sizeof(selfDuoBaoHistory.szTitle), "%s", itCfg->second.szTitle);

            if (selfDuoBaoHistory.cbState == 2)
            {
                auto itLuckyUserDetail = mapDuoBaoUserDetail.find((*itListHistory)->dwLuckyUserID);
                if (itLuckyUserDetail != mapDuoBaoUserDetail.end())
                {
                    VecDuoBaoUserInfo& vecDuoBaoUserInfo = (*itLuckyUserDetail).second;
                    std::unique_ptr<tagDuoBaoUserInfo>& duobaoUserInfoPtr = vecDuoBaoUserInfo.front();
                    if (duobaoUserInfoPtr.get() != nullptr)
                    {
                        selfDuoBaoHistory.cbLuckySex = duobaoUserInfoPtr->cbSex;
                        selfDuoBaoHistory.cbLuckyHeadIndex = duobaoUserInfoPtr->cbHeadIndex;
                    }
                }
            }
            memcpy(szBuffer+dwSendSize, &selfDuoBaoHistory, sizeof(selfDuoBaoHistory));
            dwSendSize += sizeof(selfDuoBaoHistory);
        }
    }

    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CYYDuoBaoMgr::QueryHistoryLuckyUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_DuoBao_History_LuckyUser))
    {
        return;
    }

    MSG_CS_Query_DuoBao_History_LuckyUser* pQueryHistory = (MSG_CS_Query_DuoBao_History_LuckyUser*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryHistory->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    static CT_CHAR      szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD            dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_DUOBAO_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_HISTORY_LUCKY_USER;
    msgHead.dwValue2 = pQueryHistory->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Query_DuoBao_History_LuckyUser historyLuckyUser;
    auto itHistoryData = m_mapHistoryData.find(pQueryHistory->wID);
    if (itHistoryData != m_mapHistoryData.end())
    {
        ListHistoryDuoBaoData& listHistoryDuoBaoData = itHistoryData->second;
        for (auto itListHistory = listHistoryDuoBaoData.rbegin(); itListHistory != listHistoryDuoBaoData.rend(); ++itListHistory)
        {
            auto dwIndex = (*itListHistory)->wID*DUOBAO_INDEX + (*itListHistory)->dwIndex;
            auto itHistoryUser = m_mapHistoryUser.find(dwIndex);
            if (itHistoryUser == m_mapHistoryUser.end())
            {
                continue;
            }

            CT_DWORD dwLuckyUserID = (*itListHistory)->dwLuckyUserID;
            if (dwLuckyUserID == 0)
            {
                continue;
            }

            MapDuoBaoUserDetail& mapDuoBaoUserDetail = (*itHistoryUser).second;
            auto itHistoryUserDetail = mapDuoBaoUserDetail.find((*itListHistory)->dwLuckyUserID);
            if (itHistoryUserDetail == mapDuoBaoUserDetail.end())
            {
                continue;
            }

            VecDuoBaoUserInfo& vecDuoBaoUserInfo = (*itHistoryUserDetail).second;
            std::unique_ptr<tagDuoBaoUserInfo>& duobaoUserInfoPtr = vecDuoBaoUserInfo.front();
            if (duobaoUserInfoPtr.get() == nullptr)
            {
                continue;
            }
            historyLuckyUser.wID = pQueryHistory->wID;
            historyLuckyUser.dwIndex = (*itListHistory)->dwIndex;
            historyLuckyUser.dwEndTime = (*itListHistory)->dwEndTime;
            historyLuckyUser.dwLuckyNum = (*itListHistory)->dwLuckyNum;
            historyLuckyUser.dwLuckyUser = (*itListHistory)->dwLuckyUserID;
            historyLuckyUser.cbLuckyHeadIndex = duobaoUserInfoPtr->cbHeadIndex;
            historyLuckyUser.cbLuckySex = duobaoUserInfoPtr->cbSex;
            historyLuckyUser.dwLuckyUserBuyCount = vecDuoBaoUserInfo.size();

            memcpy(szBuffer+dwSendSize, &historyLuckyUser, sizeof(MSG_SC_Query_DuoBao_History_LuckyUser));
            dwSendSize += sizeof(MSG_SC_Query_DuoBao_History_LuckyUser);
        }
    }

    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CYYDuoBaoMgr::BuyLuckyNum(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Buy_LuckyNum))
    {
        return;
    }

    MSG_CS_Buy_LuckyNum* pBuyLuckyNum = (MSG_CS_Buy_LuckyNum*)pData;
    if (pBuyLuckyNum->dwBuyCount == 0)
    {
        return;
    }

    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pBuyLuckyNum->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    //检测玩家身上的金币是否足够
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pBuyLuckyNum->dwUserID);
    if (pUserInfo == nullptr)
    {
        return;
    }

    MSG_SC_Buy_LuckyNum luckyNum;
    luckyNum.dwMainID = MSG_DUOBAO_MAIN;
    luckyNum.dwSubID = SUB_SC_BUY_LUCKY_NUM;
    luckyNum.dwValue2 = pBuyLuckyNum->dwUserID;
    luckyNum.dwIndex = pBuyLuckyNum->dwIndex;
    luckyNum.wID = pBuyLuckyNum->wID;

    //判断系统状态是否可以购买
    if (m_cbSysState == 0)
    {
        luckyNum.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    if (pUserInfo->dwGameServerID != 1000)
    {
        luckyNum.cbResult = 4;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    auto itDoingData = m_mapDoingData.find(pBuyLuckyNum->wID);
    if (itDoingData == m_mapDoingData.end())
    {
        luckyNum.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }
    std::unique_ptr<tagDuoBaoData>& duobaoDataPtr = itDoingData->second;
    if (duobaoDataPtr->cbState != 0)
    {
        luckyNum.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }
    if (duobaoDataPtr->dwIndex != pBuyLuckyNum->dwIndex)
    {
        luckyNum.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    auto itCfg = m_mapDuoBaoCfg.find(pBuyLuckyNum->wID);
    if (itCfg == m_mapDuoBaoCfg.end())
    {
        luckyNum.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    CT_DWORD dwIndex = duobaoDataPtr->wID*DUOBAO_INDEX + duobaoDataPtr->dwIndex;
    MapLuckyNum& mapLuckyNum = m_mapDoingLuckNum[dwIndex];
    CT_DWORD dwAlreadyBuyCount = (CT_DWORD)mapLuckyNum.size();
    CT_DWORD dwLeftCount = itCfg->second.dwTotalCount - dwAlreadyBuyCount;
    //剩余次数不够
    if (pBuyLuckyNum->dwBuyCount > dwLeftCount)
    {
        luckyNum.cbResult = 3;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    CT_LONGLONG llNeedScore = itCfg->second.wSingleScore * pBuyLuckyNum->dwBuyCount * TO_LL;
    if (pUserInfo->llScore < llNeedScore)
    {
        luckyNum.cbResult = 1;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }
    else if (pUserInfo->llScore < llNeedScore + itCfg->second.dwUserLeaseScore*TO_LL)
    {
        luckyNum.cbResult = 5;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    MapDuoBaoUserDetail& mapDuoBaoUserDetail = m_mapDoingUser[dwIndex];
    VecDuoBaoUserInfo& vecDuoBaoUserInfo = mapDuoBaoUserDetail[pBuyLuckyNum->dwUserID];
    if (itCfg->second.wlimitCount != 0 && (pBuyLuckyNum->dwBuyCount + vecDuoBaoUserInfo.size()) > itCfg->second.wlimitCount)
    {
        luckyNum.cbResult = 3;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));
        return;
    }

    CT_LONGLONG llSourceScore = pUserInfo->llScore;
    CT_LONGLONG llSourceBankScore = pUserInfo->llBankScore;

    //给玩家发送奖励
    CUserMgr::get_instance().AddUserScore(pBuyLuckyNum->dwUserID, -llNeedScore);

    //扣除玩家金币
    CUserMgr::get_instance().InsertUserScoreChangeRecord(pBuyLuckyNum->dwUserID, llSourceBankScore, llSourceScore, 0, -llNeedScore, BUY_DUOBAO);

    //生成幸运码
    for (CT_DWORD i = 0; i < pBuyLuckyNum->dwBuyCount; ++i)
    {
        //幸运码数据
        CT_DWORD dwLuckyNum = LUCKY_NUM + dwAlreadyBuyCount + i + 1;
        mapLuckyNum.insert(std::make_pair(dwLuckyNum, pBuyLuckyNum->dwUserID));

        //插入玩家购买数据
        std::unique_ptr<tagDuoBaoUserInfo> userInfoPtr(new tagDuoBaoUserInfo);
        userInfoPtr->wID = duobaoDataPtr->wID;
        userInfoPtr->dwIndex = duobaoDataPtr->dwIndex;
        userInfoPtr->dwUserID = pBuyLuckyNum->dwUserID;
        userInfoPtr->dwLuckyNum = dwLuckyNum;
        userInfoPtr->cbHeadIndex = pUserInfo->cbHeadID;
        userInfoPtr->cbSex = userInfoPtr->cbSex;

        InsertUserInfoToDB(userInfoPtr);
        vecDuoBaoUserInfo.push_back(std::move(userInfoPtr));
    }

    luckyNum.cbResult = 0;
    luckyNum.dwBuyCount = (CT_DWORD)vecDuoBaoUserInfo.size();
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNum, sizeof(luckyNum));

    CT_DWORD dwBuyCount = (CT_DWORD)mapLuckyNum.size();
    //广播现在的购买数量
    if (!m_setOpenUiUser.empty())
    {
        MSG_SC_Update_LuckyNum_Count luckyNumCount;
        luckyNumCount.dwMainID = MSG_DUOBAO_MAIN;
        luckyNumCount.dwSubID = SUB_SC_UPDATE_LUCKYNUM_COUNT;
        luckyNumCount.wID = pBuyLuckyNum->wID;
        luckyNumCount.dwIndex = pBuyLuckyNum->dwIndex;
        luckyNumCount.dwBuyCount = dwBuyCount;

        for (auto itOpenUser = m_setOpenUiUser.begin(); itOpenUser != m_setOpenUiUser.end(); )
        {
            acl::aio_socket_stream* pOpenUiUserSocket = CUserMgr::get_instance().FindUserProxySocksPtr(*itOpenUser);
            if (pOpenUiUserSocket == NULL)
            {
                itOpenUser = m_setOpenUiUser.erase(itOpenUser);
                continue;
            }
            luckyNumCount.dwValue2 = *itOpenUser;
            CNetModule::getSingleton().Send(pOpenUiUserSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &luckyNumCount, sizeof(luckyNumCount));
            ++itOpenUser;
        }
    }

    //判断是否够开奖
    if (dwBuyCount == itCfg->second.dwTotalCount)
    {
        //OpenOneDuoBao(duobaoDataPtr);

        duobaoDataPtr->cbState = 1;
        duobaoDataPtr->dwEndTime = time(NULL);
        UpdateDoingDuoBaoState(duobaoDataPtr, 0);
    }
}

void CYYDuoBaoMgr::CloseUi(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Close_DuoBao_UI))
    {
        return;
    }

    MSG_CS_Close_DuoBao_UI* pCloseUI = (MSG_CS_Close_DuoBao_UI*)pData;

    auto it = m_setOpenUiUser.find(pCloseUI->dwUserID);
    if (it != m_setOpenUiUser.end())
    {
        m_setOpenUiUser.erase(pCloseUI->dwUserID);

        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pCloseUI->dwUserID);
        if (pUserInfo != NULL)
        {
            if (pUserInfo->dwGameServerID == 1000)
                pUserInfo->dwGameServerID = 0;
        }
    }
}

void CYYDuoBaoMgr::QueryRewardRecord(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Duobao_Reward_Record))
    {
        return;
    }

    MSG_CS_Query_Duobao_Reward_Record* pRewardRecord = (MSG_CS_Query_Duobao_Reward_Record*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pRewardRecord->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    static CT_CHAR      szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD            dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_DUOBAO_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_REWARD_RECORD;
    msgHead.dwValue2 = pRewardRecord->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Query_DuoBao_Reward_Record selfLuckyUser;
    for (auto itHistoryData = m_mapHistoryData.begin(); itHistoryData != m_mapHistoryData.end(); ++itHistoryData)
    {
        CT_DWORD dwDuoBaoID = itHistoryData->first;
        auto itCfg = m_mapDuoBaoCfg.find(dwDuoBaoID);
        if (itCfg == m_mapDuoBaoCfg.end())
        {
            LOG(WARNING) << "duo bao id: " << dwDuoBaoID;
            continue;
        }

        ListHistoryDuoBaoData& listHistoryDuoBaoData = itHistoryData->second;
        for (auto itListHistory = listHistoryDuoBaoData.rbegin(); itListHistory != listHistoryDuoBaoData.rend(); ++itListHistory)
        {
            auto dwIndex = (*itListHistory)->wID*DUOBAO_INDEX + (*itListHistory)->dwIndex;
            auto itHistoryUser = m_mapHistoryUser.find(dwIndex);
            if (itHistoryUser == m_mapHistoryUser.end())
            {
                continue;
            }

            CT_DWORD dwLuckyUserID = (*itListHistory)->dwLuckyUserID;
            if (dwLuckyUserID != pRewardRecord->dwUserID)
            {
                continue;
            }

            MapDuoBaoUserDetail& mapDuoBaoUserDetail = (*itHistoryUser).second;
            auto itHistoryUserDetail = mapDuoBaoUserDetail.find((*itListHistory)->dwLuckyUserID);
            if (itHistoryUserDetail == mapDuoBaoUserDetail.end())
            {
                continue;
            }

            VecDuoBaoUserInfo& vecDuoBaoUserInfo = (*itHistoryUserDetail).second;
            std::unique_ptr<tagDuoBaoUserInfo>& duobaoUserInfoPtr = vecDuoBaoUserInfo.front();
            if (duobaoUserInfoPtr.get() == nullptr)
            {
                continue;
            }

            selfLuckyUser.wID = (*itListHistory)->wID;
            selfLuckyUser.dwIndex = (*itListHistory)->dwIndex;
            selfLuckyUser.dwSelfBuyCount = vecDuoBaoUserInfo.size();
            selfLuckyUser.dwLuckyNum = (*itListHistory)->dwLuckyNum;
            selfLuckyUser.dwEndTime = (*itListHistory)->dwEndTime;
            _snprintf_info(selfLuckyUser.szTitle, sizeof(selfLuckyUser.szTitle), "%s", itCfg->second.szTitle);
            memcpy(szBuffer+dwSendSize, &selfLuckyUser, sizeof(MSG_SC_Query_DuoBao_Reward_Record));
            dwSendSize += sizeof(MSG_SC_Query_DuoBao_Reward_Record);
        }
    }

    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CYYDuoBaoMgr::UpdateDataToDB(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr)
{
    //更新数据库
    MSG_CS2DB_Update_DuoBao_Data duoBaoData;
    duoBaoData.wID = duobaoDataPtr->wID;
    duoBaoData.dwIndex = duobaoDataPtr->dwIndex;
    duoBaoData.dwStartTime = duobaoDataPtr->dwStartTime;
    duoBaoData.dwEndTime = duobaoDataPtr->dwEndTime;
    duoBaoData.dwLuckyUserID = duobaoDataPtr->dwLuckyUserID;
    duoBaoData.dwLuckyNum = duobaoDataPtr->dwLuckyNum;
    duoBaoData.dwRewardScore = duobaoDataPtr->dwRewardScore;
    duoBaoData.cbState = duobaoDataPtr->cbState;

    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPADATE_DUOBAO_DATA, &duoBaoData, sizeof(MSG_CS2DB_Update_DuoBao_Data));
}

void CYYDuoBaoMgr::InsertUserInfoToDB(std::unique_ptr<tagDuoBaoUserInfo>& duobaoUserPtr)
{
    MSG_CS2DB_Insert_DuoBao_User duoBaoUser;
    duoBaoUser.wID = duobaoUserPtr->wID;
    duoBaoUser.dwIndex = duobaoUserPtr->dwIndex;
    duoBaoUser.dwUserID = duobaoUserPtr->dwUserID;
    duoBaoUser.dwLuckyNum = duobaoUserPtr->dwLuckyNum;
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_DUOBAO_USERINFO, &duoBaoUser, sizeof(MSG_CS2DB_Insert_DuoBao_User));
}