//
// Created by okman on 2019/01/14.
//

#ifndef _LUCKY_HONGBAO_MGR_H_
#define _LUCKY_HONGBAO_MGR_H_

#include "acl_cpp/lib_acl.hpp"
#include <CTType.h>
#include <GlobalStruct.h>
#include <map>
#include <CMD_Plaza.h>
#include "UserMgr.h"

class CRedPacketMgr : public acl::singleton<CRedPacketMgr>
{
public:
    CRedPacketMgr();
    ~CRedPacketMgr();

public:
    void    SetRedPacketStatus(tagRedPacketStatus* pPacketStatus);
    void    InsertRedPacketRoomCfg(tagRedPacketRoomCfg* pRedPacketRoomCfg);
    void    InsertRedPacketIndex(tagRedPacketIndex* pRedPacketIndex);
    void    InsertRedPacketGrabInfo(tagRedPacketUserGrabData* pRedPacketUserGrabData);

    void    CheckRedPacketStartEnd();
    void    SendRedPacketAward();
    void    CollectUserGameData(MSG_G2CS_PlayGame* pPlayGame);

    void    SendRedPacketDoingStatusToClient(CT_DWORD dwUserID);
    void    SendRedPacketStartToClient(CT_DWORD dwUserID);
    void    SendRedPacketStatusToClient(CT_DWORD dwUserID);
    void    RedPacketMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);
    inline void    QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void    GrabRedPacket(const CT_VOID * pData, CT_DWORD wDataSize);

private:
    inline void UpdateRedPacketStage(MSG_CS2DB_Update_RedPacket_Stage* pData);
    inline void InsertRedPacketGrabInfo(MSG_CS2DB_Insert_RadPacket_GrabInfo* pData);
    inline void SendRewardByMail(CT_BYTE* pData, CT_DWORD dwSize);
    inline bool IsSpecialTime(tm &timeNow, int nStartHour, int nStartMin, int nEndHour, int nEndMin);

private:
    struct  tagRedPacketUserGameData
    {
        CT_WORD     wGameID;
        CT_WORD     wKindID;
        CT_WORD     wRoomKindID;
        CT_DWORD    dwValue;            //局数或者捕鱼的炮数
    };

    //0: 未开始 1: 活动准备阶段 2: 活动进行中 3: 活动结束未发奖 4:活动结束已发奖
    enum en_RedPacketIndexStage
    {
        en_NotStart,
        en_Ready,
        en_Start,
        en_EndNotAward,
        en_EndAward,
    };

    typedef std::map<CT_DWORD , tagRedPacketRoomCfg>                    MapRedPacketRoomCfg;
    typedef std::map<CT_DWORD, tagRedPacketIndex>                       MapRedPacketIndex;
    typedef std::vector<tagRedPacketIndex*>                             VecRedPacketIndex;
    typedef std::map<CT_DWORD, std::vector<tagRedPacketUserGameData>>   MapRedPacketUserGameData;
    typedef std::set<CT_DWORD>                                          SetNotifyStartUser;
    typedef std::map<CT_DWORD, std::map<CT_DWORD, std::vector<tagRedPacketUserGrabData>>> MapRedPacketUserGrabData;

    MapRedPacketRoomCfg         m_mapRedPacketRoomCfg;          //红包的房间配置
    MapRedPacketIndex           m_mapRedPacketIndex;            //红包的期数配置
    VecRedPacketIndex           m_vecRedPacketIndex;            //红包的期数配置

    CT_DWORD                    m_dwRedPacketDoingIndex;        //正在进行的红包活动
    MapRedPacketUserGameData    m_mapRedPacketUserGameData;     //用户的游戏数据(每期红包活动结束时清空)
    SetNotifyStartUser          m_setNotifyStartUser;           //已经通知红包活动开始的用户(每期红包活动结束时清空)
    MapRedPacketUserGrabData    m_mapRedPacketUserGrabData;     //领取到红包的资料

    tagRedPacketStatus          m_redPacketStatus;              //红包状态
    tagRedPacketBigWinner       m_lastBigWinner;                //上期大赢家
    tagRedPacketBigWinner       m_newBigWinner;                 //本期大赢家
    bool                        m_bGenerateBigWinner;           //今天是否产生了888


    std::map<CT_BYTE, std::vector<CT_DWORD>> m_mapRedPacketScore;  //各VIP等级的红包1
    //std::map<CT_BYTE, std::vector<CT_DWORD>> m_mapRedPacketScore2;  //各VIP等级的红包2
};

#endif //_LUCKY_HONGBAO_MGR_H_