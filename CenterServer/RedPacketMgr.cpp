//
// Created by okman on 2019/01/14.
//
#include "stdafx.h"
#include "RedPacketMgr.h"
#include <timeFunction.h>
#include "NetModule.h"
#include "ServerMgr.h"
#include "SystemMsgMgr.h"
#include <Utility.h>
#include <math.h>

extern CNetConnector *pNetDB;


CRedPacketMgr::CRedPacketMgr()
:m_dwRedPacketDoingIndex(0)
,m_bGenerateBigWinner(false)
{
    /*
     * a)  VIP 0：1-3元，随机从此范围内取值（数值尾数均带8）
b)  VIP 1：3.18-6.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，3.88元、5.88元、6.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
c)  VIP 2：7.18-10.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，8.88元、9.88元、10.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
d)  VIP 3：11.18-14.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，11.88元、12.88元、14.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
e)  VIP 4：15.18-18.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，15.88元、16.88元、18.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
f)  VIP 5：19.18-22.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，19.88元、20.88元、22.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
g)  VIP 6：23.18-26.18元，输钱玩家占40%、赢钱玩家占60%概率，随机从此范围内取值，23.88元、25.88元、26.88元，输钱玩家占60%概率、赢钱玩家占40%概率的获得此3个金额中的随机一个
     */
    std::vector<CT_DWORD> vecScore;
    vecScore.push_back(128);
    vecScore.push_back(268);
    vecScore.push_back(368);
    m_mapRedPacketScore[0] = vecScore;

    vecScore.clear();
    vecScore.push_back(318);
    vecScore.push_back(418);
    vecScore.push_back(518);
    vecScore.push_back(618);

    vecScore.push_back(388);
    vecScore.push_back(588);
    vecScore.push_back(688);
    m_mapRedPacketScore[1] = vecScore;

    vecScore.clear();
    vecScore.push_back(718);
    vecScore.push_back(818);
    vecScore.push_back(918);
    vecScore.push_back(1018);

    vecScore.push_back(888);
    vecScore.push_back(988);
    vecScore.push_back(1088);
    m_mapRedPacketScore[2] = vecScore;

    vecScore.clear();
    vecScore.push_back(1118);
    vecScore.push_back(1218);
    vecScore.push_back(1318);
    vecScore.push_back(1418);

    vecScore.push_back(1188);
    vecScore.push_back(1288);
    vecScore.push_back(1488);
    m_mapRedPacketScore[3] = vecScore;

    vecScore.clear();
    vecScore.push_back(1518);
    vecScore.push_back(1618);
    vecScore.push_back(1718);
    vecScore.push_back(1818);

    vecScore.push_back(1588);
    vecScore.push_back(1688);
    vecScore.push_back(1888);
    m_mapRedPacketScore[4] = vecScore;

    vecScore.clear();
    vecScore.push_back(1918);
    vecScore.push_back(2018);
    vecScore.push_back(2118);
    vecScore.push_back(2218);

    vecScore.push_back(1988);
    vecScore.push_back(2088);
    vecScore.push_back(2288);
    m_mapRedPacketScore[5] = vecScore;

    vecScore.clear();
    vecScore.push_back(2318);
    vecScore.push_back(2418);
    vecScore.push_back(2518);
    vecScore.push_back(2618);

    vecScore.push_back(2388);
    vecScore.push_back(2588);
    vecScore.push_back(2688);
    m_mapRedPacketScore[6] = vecScore;
}

CRedPacketMgr::~CRedPacketMgr()
{

}

void    CRedPacketMgr::SetRedPacketStatus(tagRedPacketStatus* pPacketStatus)
{
    memcpy(&m_redPacketStatus, pPacketStatus, sizeof(tagRedPacketStatus));
}

void   CRedPacketMgr::InsertRedPacketRoomCfg(tagRedPacketRoomCfg* pRedPacketRoomCfg)
{
    //加载配置重复
    auto it = m_mapRedPacketRoomCfg.find(pRedPacketRoomCfg->dwGameIndex);
    if (it != m_mapRedPacketRoomCfg.end())
    {
        it->second.dwGameIndex = pRedPacketRoomCfg->dwGameIndex;
        it->second.dwRedPacketValue = pRedPacketRoomCfg->dwRedPacketValue;
        LOG(WARNING) << "update red packet room cfg, game index " << pRedPacketRoomCfg->dwGameIndex << ", value: " << pRedPacketRoomCfg->dwRedPacketValue;
        return;
    }

    //配置
    m_mapRedPacketRoomCfg[pRedPacketRoomCfg->dwGameIndex] = *pRedPacketRoomCfg;
    //LOG(WARNING) << "insert red packet room cfg, game index: " << pRedPacketRoomCfg->dwGameIndex << ", value: " << pRedPacketRoomCfg->dwRedPacketValue;
}

void   CRedPacketMgr::InsertRedPacketIndex(tagRedPacketIndex* pRedPacketIndex)
{
    //加载配置重复
    auto it = m_mapRedPacketIndex.find(pRedPacketIndex->dwIndex);
    if (it != m_mapRedPacketIndex.end())
    {
        for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
        {
            it->second.dwGameIndex[i] = pRedPacketIndex->dwGameIndex[i];
        }
        _snprintf_info(it->second.szStartDate, sizeof(it->second.szStartDate), "%s", pRedPacketIndex->szStartDate);
        it->second.cbReadyHour = pRedPacketIndex->cbReadyHour;
        it->second.cbReadyMin = pRedPacketIndex->cbReadyMin;
        it->second.cbStartHour = pRedPacketIndex->cbStartHour;
        it->second.cbStartMin = pRedPacketIndex->cbStartMin;
        it->second.cbEndHour = pRedPacketIndex->cbEndHour;
        it->second.cbEndMin = pRedPacketIndex->cbEndMin;
        it->second.dwRedPacketScore = pRedPacketIndex->dwRedPacketScore;
        it->second.cbState = pRedPacketIndex->cbState;

        LOG(WARNING) << "update red packet, index: " << pRedPacketIndex->dwIndex;
        return;
    }

    //配置
    m_mapRedPacketIndex[pRedPacketIndex->dwIndex] = *pRedPacketIndex;

    m_vecRedPacketIndex.push_back(&m_mapRedPacketIndex[pRedPacketIndex->dwIndex]);
    std::sort(m_vecRedPacketIndex.begin(), m_vecRedPacketIndex.end(), [](tagRedPacketIndex* a, tagRedPacketIndex* b)
    {
        return a->dwIndex < b->dwIndex;
    });

    //如果是正在进行的活动,需要把当前进行的Index记录下来
    if (pRedPacketIndex->cbStage == en_Ready || pRedPacketIndex->cbStage == en_Start)
    {
        m_dwRedPacketDoingIndex = pRedPacketIndex->dwIndex;
    }

    //LOG(WARNING) << "insert red packet index cfg, index: " << pRedPacketIndex->dwIndex << ", score: " << pRedPacketIndex->dwRedPacketScore;
}

void    CRedPacketMgr::InsertRedPacketGrabInfo(tagRedPacketUserGrabData* pRedPacketUserGrabData)
{
    std::map<CT_DWORD, std::vector<tagRedPacketUserGrabData>>& mapRedPacketUserGrabData = m_mapRedPacketUserGrabData[pRedPacketUserGrabData->dwIndex];
    std::vector<tagRedPacketUserGrabData>& vecUserGrabData = mapRedPacketUserGrabData[pRedPacketUserGrabData->dwUserID];
    tagRedPacketUserGrabData grabData;
    grabData.wGameID = pRedPacketUserGrabData->wGameID;
    grabData.wKindID = pRedPacketUserGrabData->wKindID;
    grabData.wRoomKindID = pRedPacketUserGrabData->wRoomKindID;
    grabData.dwIndex = pRedPacketUserGrabData->dwIndex;
    grabData.dwLuckyMoney = pRedPacketUserGrabData->dwLuckyMoney;
    grabData.dwUserID = pRedPacketUserGrabData->dwUserID;
    vecUserGrabData.push_back(grabData);
}

void    CRedPacketMgr::CheckRedPacketStartEnd()
{
    //检测红包是否开启
    if (m_redPacketStatus.cbStatus == 0)
        return;

    CT_DWORD dwNowTime = time(NULL);
    std::string strTodayDate = Utility::ChangeTimeToString(dwNowTime, "%Y-%m-%d");
    tm timeNow;
    memset(&timeNow, 0, sizeof(timeNow));
    getLocalTime(&timeNow, dwNowTime);

    //检测结束
    if (m_dwRedPacketDoingIndex != 0)
    {
        auto it = m_mapRedPacketIndex.find(m_dwRedPacketDoingIndex);
        if (it != m_mapRedPacketIndex.end())
        {
            tagRedPacketIndex& redPacketIndex = it->second;
            //检测红包是否结束
            if (redPacketIndex.cbStage == en_Ready)
            {
                bool bInTime = IsSpecialTime(timeNow, redPacketIndex.cbStartHour, redPacketIndex.cbStartMin, redPacketIndex.cbEndHour, redPacketIndex.cbEndMin);
                if (bInTime)
                {
                    redPacketIndex.cbStage = en_Start;
                    //更新阶段
                    MSG_CS2DB_Update_RedPacket_Stage redPacketStage;
                    redPacketStage.dwIndex = m_dwRedPacketDoingIndex;
                    redPacketStage.cbStage = redPacketIndex.cbStage;
                    UpdateRedPacketStage(&redPacketStage);

                    LOG(WARNING) << "red packet start, index: " << it->second.dwIndex;

                    //通知玩家红包活动开始
                    SendRedPacketStartToClient(0);
                    SendRedPacketDoingStatusToClient(0);
                }
                else
                {
                    //判断是1分钟以内广播信息
                    int nNowResult = 100 * timeNow.tm_hour + timeNow.tm_min;
                    int nStartResult = 100*redPacketIndex.cbStartHour + redPacketIndex.cbStartMin;
                    if(nStartResult > nNowResult)
                    {
                        CT_DWORD dwLeftMinStart = (redPacketIndex.cbStartHour*60+redPacketIndex.cbStartMin - (timeNow.tm_hour*60 + timeNow.tm_min));
                        if (dwLeftMinStart <= 1)
                        {
                            std::string strRoomName;
                            for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
                            {
                                if (redPacketIndex.dwGameIndex[i] != 0)
                                {
                                    CT_WORD wGameID = (CT_WORD)(redPacketIndex.dwGameIndex[i] / 10000);
                                    CT_WORD wKindID = (CT_WORD)((redPacketIndex.dwGameIndex[i] % 10000) / 100);
                                    CT_WORD wRoomKindID = (CT_WORD)((redPacketIndex.dwGameIndex[i] % 10000) % 100);
                                    strRoomName += CServerMgr::get_instance().GetGameServerName(wGameID, wKindID, wRoomKindID);
                                    strRoomName += " ";
                                }
                            }
                            CT_CHAR szMsg[1024] = {0};
                            _snprintf_info(szMsg, sizeof(szMsg), "%s红包雨即将来袭，祝您赌神庇佑，无往不利！", strRoomName.c_str());
                            CSystemMsgMgr::get_instance().SendSystemMsg(0, szMsg);
                        }
                    }
                }
            }
            else if (redPacketIndex.cbStage == en_Start)
            {
                bool bInTime = IsSpecialTime(timeNow, redPacketIndex.cbStartHour, redPacketIndex.cbStartMin, redPacketIndex.cbEndHour, redPacketIndex.cbEndMin);
                if (!bInTime)
                {
                    //生成大赢家数据
                    if (m_newBigWinner.dwBigWiner[0] == 0)
                    {
                        auto randRoomIndex = rand() % RED_PACKET_ROOM_COUNT;
                        if (redPacketIndex.dwGameIndex[randRoomIndex] == 0)
                        {
                            randRoomIndex = 0;
                        }

                        m_newBigWinner.dwIndex = m_dwRedPacketDoingIndex;
                        m_newBigWinner.wGameID[0] = (CT_WORD)(redPacketIndex.dwGameIndex[randRoomIndex] / 10000);
                        m_newBigWinner.wKindID[0] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) / 100);
                        m_newBigWinner.wRoomKindID[0] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) % 100);
                        m_newBigWinner.cbBigSex[0] = rand() % 2 + 1;
                        m_newBigWinner.cbBigHeadId[0] = rand() % 6 + 1;
                        m_newBigWinner.cbVip2[0] = rand() % 7;
                        m_newBigWinner.dwBigWiner[0] = 80000000 + rand() % 6500000;
                        m_newBigWinner.dwBigScore[0] = 88800;
                    }
                    if (m_newBigWinner.dwBigWiner[1] == 0)
                    {
                        auto randRoomIndex = rand() % RED_PACKET_ROOM_COUNT;
                        if (redPacketIndex.dwGameIndex[randRoomIndex] == 0)
                        {
                            randRoomIndex = 0;
                        }
                        m_newBigWinner.dwIndex = m_dwRedPacketDoingIndex;
                        m_newBigWinner.wGameID[1] = (CT_WORD)(redPacketIndex.dwGameIndex[randRoomIndex] / 10000);
                        m_newBigWinner.wKindID[1] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) / 100);
                        m_newBigWinner.wRoomKindID[1] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) % 100);
                        m_newBigWinner.cbBigSex[1] = rand() % 2 + 1;
                        m_newBigWinner.cbBigHeadId[1] = rand() % 6 + 1;
                        m_newBigWinner.cbVip2[1] = rand() % 7;
                        m_newBigWinner.dwBigWiner[1] = 80000000 + rand() % 6500000;
                        m_newBigWinner.dwBigScore[1] = 66600;
                    }
                    if (m_newBigWinner.dwBigWiner[2] == 0)
                    {
                        auto randRoomIndex = rand() % RED_PACKET_ROOM_COUNT;
                        if (redPacketIndex.dwGameIndex[randRoomIndex] == 0)
                        {
                            randRoomIndex = 0;
                        }
                        m_newBigWinner.dwIndex = m_dwRedPacketDoingIndex;
                        m_newBigWinner.wGameID[2] = (CT_WORD)(redPacketIndex.dwGameIndex[randRoomIndex] / 10000);
                        m_newBigWinner.wKindID[2] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) / 100);
                        m_newBigWinner.wRoomKindID[2] = (CT_WORD)((redPacketIndex.dwGameIndex[randRoomIndex] % 10000) % 100);
                        m_newBigWinner.cbBigSex[2] = rand() % 2 + 1;
                        m_newBigWinner.cbBigHeadId[2] = rand() % 6 + 1;
                        m_newBigWinner.cbVip2[2] = rand() % 7;
                        m_newBigWinner.dwBigWiner[2] = 80000000 + rand() % 6500000;
                        m_newBigWinner.dwBigScore[2] = 58800;
                    }

                    for (int i = 0; i < RED_PACKET_BIGWINNER; ++i)
                    {
                        CT_CHAR szMsg[512] = {0};
                        std::string strRoomName = CServerMgr::get_instance().GetGameServerName(m_newBigWinner.wGameID[i], m_newBigWinner.wKindID[i], m_newBigWinner.wRoomKindID[i]);
                        _snprintf_info(szMsg, sizeof(szMsg), "恭喜玩家%u在%s的抢红包活动中获得%0.2f元", m_newBigWinner.dwBigWiner[i], strRoomName.c_str(), m_newBigWinner.dwBigScore[i]*TO_DOUBLE);
                        CSystemMsgMgr::get_instance().SendSystemMsg(0, szMsg);
                    }

                    memcpy(&m_lastBigWinner, &m_newBigWinner, sizeof(tagRedPacketBigWinner));
                    m_newBigWinner.Reset();

                    //通知红包活动结束
                    redPacketIndex.cbStage = en_EndNotAward;
                    SendRedPacketDoingStatusToClient(0);

                    //更新阶段
                    MSG_CS2DB_Update_RedPacket_Stage redPacketStage;
                    redPacketStage.dwIndex = m_dwRedPacketDoingIndex;
                    redPacketStage.cbStage = redPacketIndex.cbStage;
                    UpdateRedPacketStage(&redPacketStage);

                    //重置数据
                    m_dwRedPacketDoingIndex = 0;
                    m_mapRedPacketUserGameData.clear();
                    m_setNotifyStartUser.clear();

                    LOG(WARNING) << "red packet end no award, index: " << it->second.dwIndex;
                }
            }
        }
    }

    //检测新活动开始
    for (auto& it : m_vecRedPacketIndex)
    {
        //活动已经关闭不检测
        if (it->cbState == 0 || strcmp(strTodayDate.c_str(), it->szStartDate) != 0)
            continue;

        //检测活动是否进入准备阶段
        if (it->cbStage == en_NotStart)
        {
            bool bReady = IsSpecialTime(timeNow, it->cbReadyHour, it->cbReadyMin, it->cbStartHour, it->cbStartMin);
            if (bReady)
            {
                m_dwRedPacketDoingIndex = it->dwIndex;
                it->cbStage = en_Ready;

                //更新阶段
                MSG_CS2DB_Update_RedPacket_Stage redPacketStage;
                redPacketStage.dwIndex = it->dwIndex;
                redPacketStage.cbStage =  it->cbStage;
                UpdateRedPacketStage(&redPacketStage);
                LOG(WARNING) << "red packet ready, index: " << it->dwIndex;
                break;
            }
            else
            {
                //判断是1分钟以内广播信息
                int nNowResult = 100 * timeNow.tm_hour + timeNow.tm_min;
                int nStartResult = 100*it->cbReadyHour + it->cbReadyMin;
                if(nStartResult > nNowResult)
                {
                    CT_DWORD dwLeftMinStart = (it->cbReadyHour*60+it->cbReadyMin - (timeNow.tm_hour*60 + timeNow.tm_min));
                    if (dwLeftMinStart <= 1)
                    {
                        std::string strRoomName;
                        for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
                        {
                            if (it->dwGameIndex[i] != 0)
                            {
                                CT_WORD wGameID = (CT_WORD)(it->dwGameIndex[i] / 10000);
                                CT_WORD wKindID = (CT_WORD)((it->dwGameIndex[i] % 10000) / 100);
                                CT_WORD wRoomKindID = (CT_WORD)((it->dwGameIndex[i] % 10000) % 100);
                                strRoomName += CServerMgr::get_instance().GetGameServerName(wGameID, wKindID, wRoomKindID);
                                strRoomName += " ";
                            }
                        }
                        CT_CHAR szMsg[512] = {0};
                        _snprintf_info(szMsg, sizeof(szMsg), "财神红包将在%02d:%02d:00在 %s掉落！现在进入房间即可累计局数，祝您好运连连！", it->cbStartHour, it->cbStartMin, strRoomName.c_str());
                        CSystemMsgMgr::get_instance().SendSystemMsg(0, szMsg);
                    }
                }
            }
        }
    }
}

void    CRedPacketMgr::SendRedPacketAward()
{
    if (m_redPacketStatus.cbStatus == 0)
    {
        return;
    }

    //重置
    if (m_bGenerateBigWinner)
        m_bGenerateBigWinner = false;

    //检测今天完成的活动,邮件发送奖励
    for (auto& it : m_vecRedPacketIndex)
    {
        if (it->cbStage != en_EndNotAward)
            continue;

        CT_DWORD dwTimeNow = Utility::GetTime();
        std::string strTodayDate = Utility::ChangeTimeToString(dwTimeNow, "%Y-%m-%d");

        //当天的红包不发送
        if (strcmp(strTodayDate.c_str(), it->szStartDate) == 0)
            continue;

        auto itRedPacketGrabData = m_mapRedPacketUserGrabData.find(it->dwIndex);
        if (itRedPacketGrabData != m_mapRedPacketUserGrabData.end())
        {
            //CT_BYTE  szBuffer[SYS_NET_SENDBUF_CLIENT];
            //CT_DWORD dwSendSize = 0;

            std::map<CT_DWORD, std::vector<tagRedPacketUserGrabData>>& mapUserGrabData = itRedPacketGrabData->second;
            for (auto& itMapUserGrabData : mapUserGrabData)
            {
                std::vector<tagRedPacketUserGrabData>& vecUserGrabData = itMapUserGrabData.second;
                for (auto& itVecUserGrabData : vecUserGrabData)
                {
                    /*if (dwSendSize + sizeof(tagUserMail) >= SYS_NET_SENDBUF_CLIENT)
                    {
                        //缓存爆满，先把部分数据发送到dbserver
                        SendRewardByMail(szBuffer, dwSendSize);
                        dwSendSize = 0;
                    }*/

                    tagUserMail mail;
                    mail.cbState = 0;
                    mail.cbMailType = 2;
                    mail.dwSendTime = dwTimeNow;
                    mail.dwExpiresTime = mail.dwSendTime + 89600 * 30; //默认30天的有效期
                    mail.dwUserID = itMapUserGrabData.first;
                    mail.llScore = itVecUserGrabData.dwLuckyMoney;
                    mail.cbScoreChangeType = GET_HONGBAO;
                    _snprintf_info(mail.szTitle, sizeof(mail.szTitle), "红包奖励");

                    std::string gameServerName =CServerMgr::get_instance().GetGameServerName(itVecUserGrabData.wGameID,
                        itVecUserGrabData.wKindID, itVecUserGrabData.wRoomKindID);
                    _snprintf_info(mail.szContent, sizeof(mail.szContent), "恭喜您在红包活动第%d期%s抢到%.2f金币",
                        it->dwIndex, gameServerName.c_str(), itVecUserGrabData.dwLuckyMoney*TO_DOUBLE);

                    //发送邮件消息提醒
                    CUserMgr::get_instance().SendNewMailTip(itMapUserGrabData.first);
                    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, &mail,
                                                    sizeof(tagUserMail));

                    //memcpy(szBuffer + dwSendSize, &mail, sizeof(mail));
                    //dwSendSize+= sizeof(mail);
                }
            }

            /*if (dwSendSize > 0)
            {
                SendRewardByMail(szBuffer, dwSendSize);
            }*/

            //删除抢到的红包信息
            m_mapRedPacketUserGrabData.erase(itRedPacketGrabData);
        }

        //标识这期活动结束
        it->cbStage = en_EndAward;
        //更新阶段
        MSG_CS2DB_Update_RedPacket_Stage redPacketStage;
        redPacketStage.dwIndex = it->dwIndex;
        redPacketStage.cbStage =  it->cbStage;
        UpdateRedPacketStage(&redPacketStage);
    }
}

void    CRedPacketMgr::CollectUserGameData(MSG_G2CS_PlayGame* pPlayGame)
{
    //LOG(WARNING) << "CollectUserGameData, user id: " << pPlayGame->dwUserID;
    if (m_dwRedPacketDoingIndex != 0)
    {
        auto itIndex = m_mapRedPacketIndex.find(m_dwRedPacketDoingIndex);
        if (itIndex == m_mapRedPacketIndex.end())
            return;

        CT_DWORD dwGameIndex = pPlayGame->wGameID*10000+pPlayGame->wKindID*100+pPlayGame->wRoomKindID;
        for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
        {
            if (itIndex->second.dwGameIndex[i] != 0)
            {
                auto itGameRoom = m_mapRedPacketRoomCfg.find(itIndex->second.dwGameIndex[i]);
                if (itGameRoom != m_mapRedPacketRoomCfg.end()
                && itGameRoom->second.dwGameIndex == dwGameIndex)
                {
                    CT_DWORD dwValue = 0;
                    if (pPlayGame->wGameID != GAME_FISH)
                        dwValue = 1;
                    else
                        dwValue = pPlayGame->dwJettonScore;//捕鱼把这个字段当成炮数用


                    std::vector<tagRedPacketUserGameData>& vecRedPacketUserGameData = m_mapRedPacketUserGameData[pPlayGame->dwUserID];
                    for (auto& it : vecRedPacketUserGameData)
                    {
                        if (it.wGameID == pPlayGame->wGameID && it.wKindID == pPlayGame->wKindID && it.wRoomKindID == pPlayGame->wRoomKindID)
                        {
                            it.dwValue += dwValue;
                            //LOG(WARNING) << "add game data, value: " <<  it.dwValue;
                            return;
                        }
                    }
                    tagRedPacketUserGameData gameData;
                    gameData.wGameID = pPlayGame->wGameID;
                    gameData.wKindID = pPlayGame->wKindID;
                    gameData.wRoomKindID = pPlayGame->wRoomKindID;
                    gameData.dwValue = dwValue;
                    vecRedPacketUserGameData.push_back(gameData);
                    //LOG(WARNING) << "new game data, value: " <<  dwValue;
                    return;
                }
            }
        }
    }
}

void   CRedPacketMgr::SendRedPacketDoingStatusToClient(CT_DWORD dwUserID)
{
    if (m_dwRedPacketDoingIndex == 0 || m_redPacketStatus.cbStatus == 0)
    {
        return;
    }

    CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT];
    CT_DWORD dwSendSize = 0;

    MSG_SC_RedPacket_GameRoom_Status gameRoomStatus;
    gameRoomStatus.dwRedPacketIndex = m_dwRedPacketDoingIndex;
    gameRoomStatus.dwMainID = MSG_REDPACKET_MAIN;
    gameRoomStatus.dwSubID = SUB_SC_REDPACKET_GAMEROOM_STATUS;

    auto it = m_mapRedPacketIndex.find(m_dwRedPacketDoingIndex);
    if (it != m_mapRedPacketIndex.end())
    {
        tagRedPacketIndex &redPacketIndex = it->second;
        gameRoomStatus.cbStatus = (CT_BYTE)(redPacketIndex.cbStage == en_Start ? 1 : 2);

        if (redPacketIndex.cbStage == en_Start)
        {
            MSG_SC_RedPacket_Room redPacketRoom;
            for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
            {
                redPacketRoom.wGameID = (CT_WORD)(redPacketIndex.dwGameIndex[i] / 10000);
                redPacketRoom.wKindID = (CT_WORD)((redPacketIndex.dwGameIndex[i] % 10000) / 100);
                redPacketRoom.wRoomKindID = (CT_WORD)((redPacketIndex.dwGameIndex[i] % 10000) % 100);
                memcpy(szBuffer + dwSendSize + sizeof(MSG_SC_RedPacket_GameRoom_Status), &redPacketRoom, sizeof(MSG_SC_RedPacket_Room));
                dwSendSize += sizeof(MSG_SC_RedPacket_Room);
            }
        }
    }

    if (dwUserID != 0)
    {
        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
        if (pUserInfo == NULL || !pUserInfo->bOnline)
        {
            LOG(WARNING) << "can not find user.";
            return;
        }

        acl::aio_socket_stream* pSocket =  CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (pSocket == NULL)
        {
            LOG(WARNING) << "can not find proxy server.";
            return;
        }

        gameRoomStatus.dwValue2 = dwUserID;
        memcpy(szBuffer, &gameRoomStatus, sizeof(MSG_SC_RedPacket_GameRoom_Status));
        dwSendSize += sizeof(MSG_SC_RedPacket_GameRoom_Status);

        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
        //LOG(WARNING) << "send status to client, user id: " << dwUserID;
    }
    else
    {
        MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto& it : mapAllGlobalUser)
        {
            acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
            if (pSocket == NULL)
            {
                continue;
            }

            gameRoomStatus.dwValue2 = it.first;
            memcpy(szBuffer, &gameRoomStatus, sizeof(MSG_SC_RedPacket_GameRoom_Status));
            dwSendSize += sizeof(MSG_SC_RedPacket_GameRoom_Status);
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
            dwSendSize -= sizeof(MSG_SC_RedPacket_GameRoom_Status);
            //LOG(WARNING) << "send status to all client, user id: " << it.first;
        }
    }
}

void CRedPacketMgr::SendRedPacketStartToClient(CT_DWORD dwUserID)
{
    if (m_dwRedPacketDoingIndex == 0)
        return;

    auto it = m_mapRedPacketIndex.find(m_dwRedPacketDoingIndex);
    if (it == m_mapRedPacketIndex.end())
    {
        return;
    }

    if (it->second.cbStage != en_Start)
        return;

    //已经发送过红包活动开始的玩家不再发送
    if (dwUserID != 0)
    {
        auto itNotify = m_setNotifyStartUser.find(dwUserID);
        if (itNotify != m_setNotifyStartUser.end())
        {
            return;
        }
    }

    tagRedPacketIndex& redPacketIndex = it->second;
    MSG_SC_RedPacket_Start redPacketStart;
    redPacketStart.dwMainID = MSG_REDPACKET_MAIN;
    redPacketStart.dwSubID = SUB_SC_REEPACKET_START;
    //redPacketStart.dwRedPacketIndex = m_dwRedPacketDoingIndex;

    if (dwUserID == 0)
    {
        MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto& itUser: mapAllGlobalUser)
        {
            std::shared_ptr<CGlobalUser>& GlobalUserPtr = itUser.second;
            tagGlobalUserInfo* pUserInfo = GlobalUserPtr->GetUserInfo();
            if (pUserInfo == NULL || !pUserInfo->bOnline)
            {
                continue;
            }

            CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(pUserInfo->dwGameServerID);
            if (pGameServerInfo == NULL)
            {
                continue;
            }

            acl::aio_socket_stream* pSocket =  CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
            if (pSocket == NULL)
            {
                continue;
            }

            CT_DWORD dwGameIndex = (CT_DWORD)pGameServerInfo->wGameID*10000+pGameServerInfo->wKindID*100+pGameServerInfo->wRoomKindID;
            for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
            {
                if (redPacketIndex.dwGameIndex[i] == dwGameIndex)
                {
                    redPacketStart.wGameID = pGameServerInfo->wGameID;
                    redPacketStart.wKindID = pGameServerInfo->wKindID;
                    redPacketStart.wRoomKindID = pGameServerInfo->wRoomKindID;
                    redPacketStart.dwValue2 = itUser.first;
                    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &redPacketStart, sizeof(MSG_SC_RedPacket_Start));
                    m_setNotifyStartUser.insert(itUser.first);
                    //LOG(WARNING) << "send red packet start to client, user id: " <<  itUser.first;
                    break;
                }
            }
        }
    }
    else
    {
        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
        if (pUserInfo == NULL || !pUserInfo->bOnline)
        {
            return;
        }

        CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(pUserInfo->dwGameServerID);
        if (pGameServerInfo == NULL)
        {
            return;
        }

        acl::aio_socket_stream* pSocket =  CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (pSocket == NULL)
        {
            return;
        }

        CT_DWORD dwGameIndex = (CT_DWORD)pGameServerInfo->wGameID*10000+pGameServerInfo->wKindID*100+pGameServerInfo->wRoomKindID;
        for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
        {
            if (redPacketIndex.dwGameIndex[i] == dwGameIndex)
            {
                redPacketStart.wGameID = pGameServerInfo->wGameID;
                redPacketStart.wKindID = pGameServerInfo->wKindID;
                redPacketStart.wRoomKindID = pGameServerInfo->wRoomKindID;
                redPacketStart.dwValue2 = dwUserID;
                CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &redPacketStart, sizeof(MSG_SC_RedPacket_Start));
                m_setNotifyStartUser.insert(dwUserID);
                //LOG(WARNING) << "send red packet start to client, user id: " <<  dwUserID;
                break;
            }
        }
    }
}

void CRedPacketMgr::SendRedPacketStatusToClient(CT_DWORD dwUserID)
{
    if (dwUserID == 0)
    {
        MSG_SC_RedPacket_Status redPacketStatus;
        redPacketStatus.cbStatus = m_redPacketStatus.cbStatus;
        _snprintf_info(redPacketStatus.szDes, sizeof(redPacketStatus.szDes), "%s", m_redPacketStatus.szDes);
        redPacketStatus.dwMainID = MSG_REDPACKET_MAIN;
        redPacketStatus.dwSubID = SUB_SC_REDPACKET_STATUS;
        MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto& it : mapAllGlobalUser)
        {
            acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
            if (pSocket == NULL)
            {
                continue;
            }

            redPacketStatus.dwValue2 = it.first;
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &redPacketStatus, sizeof(MSG_SC_RedPacket_Status));
        }
    }
    else
    {
        if (m_redPacketStatus.cbStatus == 0)
        {
            return;
        }


        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
        if (pUserInfo == NULL || !pUserInfo->bOnline)
        {
            return;
        }

        acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (pSocket == NULL)
        {
            return;
        }

        MSG_SC_RedPacket_Status redPacketStatus;
        //三个小时弹框一次
        if (m_redPacketStatus.cbStatus == 1 && pUserInfo->dwLastHeartBeatTime - pUserInfo->dwLastRedPacketTime < 10800)
        {
            redPacketStatus.cbStatus = 2;
        }
        else
        {
            redPacketStatus.cbStatus = m_redPacketStatus.cbStatus;
        }
        pUserInfo->dwLastRedPacketTime = pUserInfo->dwLastHeartBeatTime;
        _snprintf_info(redPacketStatus.szDes, sizeof(redPacketStatus.szDes), "%s", m_redPacketStatus.szDes);
        redPacketStatus.dwMainID = MSG_REDPACKET_MAIN;
        redPacketStatus.dwSubID = SUB_SC_REDPACKET_STATUS;
        redPacketStatus.dwValue2 = dwUserID;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &redPacketStatus, sizeof(MSG_SC_RedPacket_Status));
    }
}


void    CRedPacketMgr::RedPacketMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
    switch(wSubCmdID)
    {
        case SUB_CS_REDPACKET_MAIN_INFO:
        {
            QueryMainInfo(pData, wDataSize);
            break;
        }

        case SUB_CS_REDPACKET_GRAB_LUCKY_MONEY:
        {
            GrabRedPacket(pData, wDataSize);
            break;
        }
        default:break;
    }
}

void CRedPacketMgr::QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (m_redPacketStatus.cbStatus == 0)
    {
        return;
    }

    if (wDataSize != sizeof(MSG_CS_RedPacket_Main))
    {
        return;
    }

    MSG_CS_RedPacket_Main* pQueryMain = (MSG_CS_RedPacket_Main*)pData;
    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pQueryMain->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    CT_CHAR     szBuffer[SYS_NET_SENDBUF_CLIENT] = {0};
    CT_DWORD    dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_REDPACKET_MAIN;
    msgHead.dwSubID = SUB_SC_REDPACKET_MAIN_INFO;
    msgHead.dwValue2 = pQueryMain->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_RedPacket_Main redPacketMain;

    /*CT_CHAR 	szStartDate[DATE_LEN];					//开始日期
    CT_CHAR     szStartTime[TIME_LEN_2];                //开始时间
    CT_CHAR     szDes[RED_PACKET_DES];                  //红包说明
    CT_DWORD    dwLastIndex;                            //上一期ID
    CT_BYTE     cbBigSex[RED_PACKET_BIGWINNER];         //上一期大赢家性别
    CT_BYTE     cbBigHeadId[RED_PACKET_BIGWINNER];      //上一期大赢头像
    CT_BYTE     cbVip2[RED_PACKET_BIGWINNER];           //上一期大赢VIP2
    CT_DWORD	dwBigWiner[RED_PACKET_BIGWINNER];       //上一期大赢家
    CT_DOUBLE   dBigScore[RED_PACKET_BIGWINNER];        //上一期大赢家金币
    CT_BYTE     cbGameRoomCount;                        //正在进行或将进行的红包活动房间数量*/
   // _snprintf_info(redPacketMain.szDes, sizeof(redPacketMain.szDes), "%s", m_redPacketStatus.szDes);
    redPacketMain.dwLastIndex = m_lastBigWinner.dwIndex;
    for (int i = 0; i < RED_PACKET_BIGWINNER; ++i)
    {
        redPacketMain.cbBigSex[i] = m_lastBigWinner.cbBigSex[i];
        redPacketMain.cbVip2[i] = m_lastBigWinner.cbVip2[i];
        redPacketMain.cbBigHeadId[i] = m_lastBigWinner.cbBigHeadId[i];
        redPacketMain.dwBigWiner[i] = m_lastBigWinner.dwBigWiner[i];
        redPacketMain.dBigScore[i] = m_lastBigWinner.dwBigScore[i]*TO_DOUBLE;
    }
    for (auto& it : m_vecRedPacketIndex)
    {
        if (it->cbState == 0)
            continue;

        if (it->cbStage == en_NotStart || it->cbStage == en_Ready || it->cbStage == en_Start)
        {
            _snprintf_info(redPacketMain.szStartDate, sizeof(redPacketMain.szStartDate), "%s", it->szStartDate);
            _snprintf_info(redPacketMain.szStartTime, sizeof(redPacketMain.szStartTime), "%02d:%02d:00", it->cbStartHour, it->cbStartMin);

            MSG_SC_RedPacket_Room redPacketRoom;
            for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
            {
                auto itRoom = m_mapRedPacketRoomCfg.find(it->dwGameIndex[i]);
                if (itRoom != m_mapRedPacketRoomCfg.end())
                {
                    redPacketRoom.wGameID = (CT_WORD)(itRoom->second.dwGameIndex / 10000);
                    redPacketRoom.wKindID = (CT_WORD)((itRoom->second.dwGameIndex % 10000) / 100);
                    redPacketRoom.wRoomKindID = (CT_WORD)((itRoom->second.dwGameIndex % 10000) % 100);
                    memcpy(szBuffer + dwSendSize + sizeof(MSG_SC_RedPacket_Main), &redPacketRoom, sizeof(MSG_SC_RedPacket_Room));
                    dwSendSize += sizeof(MSG_SC_RedPacket_Room);
                    ++redPacketMain.cbGameRoomCount;
                }
            }
            break;
        }
    }
    memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &redPacketMain, sizeof(MSG_SC_RedPacket_Main));
    dwSendSize += sizeof(MSG_SC_RedPacket_Main);

    //发送数据
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

void CRedPacketMgr::GrabRedPacket(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_RedPacket_Grab_LuckyMoney))
    {
        return;
    }

    MSG_CS_RedPacket_Grab_LuckyMoney* pGrabLuckyMoney = (MSG_CS_RedPacket_Grab_LuckyMoney*)pData;

    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGrabLuckyMoney->dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    tagGlobalUserInfo* pUserinfo = CUserMgr::get_instance().GetUserInfo(pGrabLuckyMoney->dwUserID);
    if (pUserinfo == NULL)
    {
        return;
    }

    MSG_SC_RedPacket_Grab_LuckyMoney grabLuckyMoney;
    grabLuckyMoney.dwMainID = MSG_REDPACKET_MAIN;
    grabLuckyMoney.dwSubID = SUB_SC_REDPACKET_GRAB_LUCKY_MONEY;
    grabLuckyMoney.dwValue2 = pGrabLuckyMoney->dwUserID;

    //检测活动是否结束
    if (pGrabLuckyMoney->dwRedPacketIndex != m_dwRedPacketDoingIndex)
    {
        grabLuckyMoney.cbResult = 3;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
        return;
    }

    //检测玩家之前是否领取红包
    std::map<CT_DWORD, std::vector<tagRedPacketUserGrabData>>& mapRedPacketUserGrabData = m_mapRedPacketUserGrabData[m_dwRedPacketDoingIndex];
    auto itGrabData = mapRedPacketUserGrabData.find(pGrabLuckyMoney->dwUserID);
    if (itGrabData != mapRedPacketUserGrabData.end())
    {
        std::vector<tagRedPacketUserGrabData>& vecGrabData = itGrabData->second;
        for (auto& itUserGrab : vecGrabData)
        {
            if (itUserGrab.wGameID == pGrabLuckyMoney->wGameID
                &&  itUserGrab.wKindID == pGrabLuckyMoney->wKindID
                &&  itUserGrab.wRoomKindID == pGrabLuckyMoney->wRoomKindID)
            {
                grabLuckyMoney.cbResult = 4;
                grabLuckyMoney.dLuckyMoney = itUserGrab.dwLuckyMoney*TO_DOUBLE;
                CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
                return;
            }
        }
    }

    //检测红包是否已经领取完
    auto itIndex = m_mapRedPacketIndex.find(m_dwRedPacketDoingIndex);
    if (itIndex == m_mapRedPacketIndex.end())
    {
        grabLuckyMoney.cbResult = 3;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
        return;
    }
    tagRedPacketIndex& redPacketIndex = itIndex->second;
    if (redPacketIndex.dwRedRacketGrabbedScore >= redPacketIndex.dwRedPacketScore)
    {
        grabLuckyMoney.cbResult = 3;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
        return;
    }

    CT_DWORD dwNeedValue = 0;
    CT_DWORD dwGameIndex = pGrabLuckyMoney->wGameID*10000+pGrabLuckyMoney->wKindID*100+pGrabLuckyMoney->wRoomKindID;
    //LOG(WARNING) << "grab lucky money, gameid : " <<  pGrabLuckyMoney->wGameID << ", kind id: " << pGrabLuckyMoney->wKindID << ", room kind id: " << pGrabLuckyMoney->wRoomKindID;
    auto itRoom = m_mapRedPacketRoomCfg.find(dwGameIndex);
    if (itRoom == m_mapRedPacketRoomCfg.end())
    {
        grabLuckyMoney.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
        //LOG(WARNING) << "grab lucky money, gameid : " <<  pGrabLuckyMoney->wGameID << ", kind id: " << pGrabLuckyMoney->wKindID << ", room kind id: " << pGrabLuckyMoney->wRoomKindID;
        return;
    }
    dwNeedValue = itRoom->second.dwRedPacketValue;

    //检测玩家是否达到条件
    bool bFinish = (dwNeedValue == 0 ? true : false);
    if (dwNeedValue != 0)
    {
        auto itGameData = m_mapRedPacketUserGameData.find(pGrabLuckyMoney->dwUserID);
        if (itGameData == m_mapRedPacketUserGameData.end())
        {
            grabLuckyMoney.cbResult = 2;
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
            return;
        }
        std::vector<tagRedPacketUserGameData>& vecRedPacketUserGameData = itGameData->second;
        for (auto& itUserGameData : vecRedPacketUserGameData)
        {
            if (itUserGameData.wGameID == pGrabLuckyMoney->wGameID
                &&  itUserGameData.wKindID == pGrabLuckyMoney->wKindID
                &&  itUserGameData.wRoomKindID == pGrabLuckyMoney->wRoomKindID)
            {
                if (itUserGameData.dwValue >= dwNeedValue)
                {
                    bFinish = true;
                    break;
                }
            }
        }
    }

    //抢红包逻辑
    if (bFinish)
    {
        auto itScore = m_mapRedPacketScore.find(pUserinfo->cbVip2);
        if (itScore == m_mapRedPacketScore.end())
        {
            grabLuckyMoney.cbResult = 3;
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
            return;
        }

        //计算红包剩余金额
        CT_DWORD dwRedPacketLeftScore = redPacketIndex.dwRedPacketScore - redPacketIndex.dwRedRacketGrabbedScore;

        CT_DWORD dwLuckyMoney = 0;
        //如果今天还没有产生888，则看看能不能
        if (!m_bGenerateBigWinner && dwRedPacketLeftScore >= 88800)
        {
            //千分之二的机会
            auto nRandNum = rand() % 1000;
            if (nRandNum <= 2)
            {
                m_bGenerateBigWinner = true;
                dwLuckyMoney = 88800;

                m_newBigWinner.dwIndex = m_dwRedPacketDoingIndex;
                m_newBigWinner.wGameID[0] = pGrabLuckyMoney->wGameID;
                m_newBigWinner.wKindID[0] = pGrabLuckyMoney->wKindID;
                m_newBigWinner.wRoomKindID[0] = pGrabLuckyMoney->wRoomKindID;
                m_newBigWinner.cbBigSex[0] = pUserinfo->cbSex;
                m_newBigWinner.cbBigHeadId[0] = pUserinfo->cbHeadID;
                m_newBigWinner.cbVip2[0] = pUserinfo->cbVip2;
                m_newBigWinner.dwBigWiner[0] = pUserinfo->dwUserID;
                m_newBigWinner.dwBigScore[0] = dwLuckyMoney;
            }
            else
            {
                std::vector<CT_DWORD>& vecLuckyMoney = itScore->second;
                auto nRandIndex = rand() % vecLuckyMoney.size();
                dwLuckyMoney = vecLuckyMoney[nRandIndex];
            }
        }
        else
        {
            std::vector<CT_DWORD>& vecLuckyMoney = itScore->second;
            auto nRandIndex = rand() % vecLuckyMoney.size();
            dwLuckyMoney = vecLuckyMoney[nRandIndex];
        }

        //红包金币增加
        redPacketIndex.dwRedRacketGrabbedScore += dwLuckyMoney;

        //玩家抢红包记录
        std::vector<tagRedPacketUserGrabData>& vecUserGrabData = mapRedPacketUserGrabData[pGrabLuckyMoney->dwUserID];
        tagRedPacketUserGrabData grabData;
        grabData.wGameID = pGrabLuckyMoney->wGameID;
        grabData.wKindID = pGrabLuckyMoney->wKindID;
        grabData.wRoomKindID = pGrabLuckyMoney->wRoomKindID;
        grabData.dwIndex = m_dwRedPacketDoingIndex;
        grabData.dwLuckyMoney = dwLuckyMoney;
        grabData.dwUserID = pGrabLuckyMoney->dwUserID;
        vecUserGrabData.push_back(grabData);

        //玩家抢红包数据库记录
        MSG_CS2DB_Insert_RadPacket_GrabInfo grabInfo;
        grabInfo.dwIndex = m_dwRedPacketDoingIndex;
        grabInfo.dwUserID = pGrabLuckyMoney->dwUserID;
        grabInfo.cbIsRobot = 0;
        grabInfo.wGameID = pGrabLuckyMoney->wGameID;
        grabInfo.wKindID = pGrabLuckyMoney->wKindID;
        grabInfo.wRoomKindID = pGrabLuckyMoney->wRoomKindID;
        grabInfo.dwIndex = m_dwRedPacketDoingIndex;
        grabInfo.dwLuckyMoney = dwLuckyMoney;
       InsertRedPacketGrabInfo(&grabInfo);

        grabLuckyMoney.cbResult = 1;
        grabLuckyMoney.dLuckyMoney = dwLuckyMoney * TO_DOUBLE;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
    }
    else
    {
        grabLuckyMoney.cbResult = 2;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &grabLuckyMoney, sizeof(grabLuckyMoney));
        return;
    }
}

void CRedPacketMgr::UpdateRedPacketStage(MSG_CS2DB_Update_RedPacket_Stage* pData)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_REDPACKET_STAGE, pData, sizeof(MSG_CS2DB_Update_RedPacket_Stage));
}

void CRedPacketMgr::InsertRedPacketGrabInfo(MSG_CS2DB_Insert_RadPacket_GrabInfo* pData)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_REDPACKET_GRAB_INFO, pData, sizeof(MSG_CS2DB_Insert_RadPacket_GrabInfo));
}

void CRedPacketMgr::SendRewardByMail(CT_BYTE* pData, CT_DWORD dwSize)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, pData, dwSize);
}

bool CRedPacketMgr::IsSpecialTime(tm &timeNow, int nStartHour, int nStartMin, int nEndHour, int nEndMin)
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