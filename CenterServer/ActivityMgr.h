//
// Created by luwei on 2018/5/28.
//

#ifndef WORKSPACE_ACTIVITYMGR_H
#define WORKSPACE_ACTIVITYMGR_H

#include "acl_cpp/lib_acl.hpp"
#include <CTType.h>
#include <GlobalStruct.h>
#include <map>
#include <CMD_Plaza.h>
#include "UserMgr.h"

//所有财神降临配置
//<--活动id, --tagWealthGodComingCfg>
typedef std::map<CT_WORD, tagWealthGodComingCfg> MapWealthGodComingCfg;
//<--gameid*100+kindid*10+roomkind, std::vector<tagWealthGodComingCfg>>
typedef std::map<CT_DWORD, std::vector<tagWealthGodComingCfg>> MapWealthGodComingCfg2;

//财神降临奖励配置
//<--gameid*100+kindid*10+roomkind, --名次id,--tagWealthGodComingReward>
typedef std::map<CT_DWORD, std::map<CT_BYTE, tagWealthGodComingReward>> MapWealthGodComingReward;

//财神降临场
//<--gameid*100+kindid*10+roomkind, --tagWealthGodComingLastIndex>
typedef std::map<CT_DWORD, tagWealthGodComingLastIndex> MapWealthGodComingLastIndex;

//正在进行的财神降临活动<--活动id, tagWealthGodComingData>
typedef std::map<CT_WORD, tagWealthGodComingData> MapDoingWealthGodComing;

//正在进行的财神降临活动
// <----gameid*100+kindid*10+roomkind, --活动id>
typedef std::map<CT_DWORD , CT_WORD > MapDoingWealthGodComingIndex;

//正在进行的财神降临活动的玩家参与信息
//<----gameid*100+kindid*10+roomkind, --userid, --tagWealthGodComingUserInfo>
typedef std::map<CT_DWORD, std::map<CT_DWORD, tagWealthGodComingUserInfo>> MapWealthGodComingUserInfo;
typedef std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo*>> MapWealthGodComingUserInfo2;

//历史财神降临活动的排名信息
// <--gameid*100+kindid*10+roomkind，--场次id, std::vector<tagWealthGodComingUserInfo>>
typedef std::map<CT_DWORD, std::map<CT_DWORD, std::vector<tagWealthGodComingUserInfo>>> MapHistoryWealthGodComingUserInfo;

//历史财神活动的活动信息
// <--gameid*100+kindid*10+roomkind, --场次id, tagWealthGodComingData>
typedef std::map<CT_DWORD,std::map<CT_DWORD, tagWealthGodComingData>> MapHistoryWealthGodComingData;

class CActivityMgr : public acl::singleton<CActivityMgr>
{
public:
    CActivityMgr();
    ~CActivityMgr();

public:
    void InsertWealthGodComing(tagWealthGodComingCfg* pWealthGodComing);
    void InsertWealthGodComingReward(tagWealthGodComingReward* pReward);
    void UpdateWealthGodComing(tagWealthGodComingCfg* pWealthGodComing);
    void InsertWealthGodComingIndex(tagWealthGodComingLastIndex* pIndex);
    void InsertWealthGodComingHistoryData(tagWealthGodComingData* pWealthGodComingData);
    void InsertWealthGodComingHistoryUser(tagWealthGodComingUserInfoEx* pWealthGodComingUser);

public:
    //定时器检测活动开始和结束
    void CheckWealthGodComingStartEnd();
    //玩家参与财神降临活动
    void UserCheckDoingWealthGodComing(MSG_G2CS_PlayGame* pPlayGame);
    //计算活动玩家排名
    void CalWealthGodComingRank(tagWealthGodComingData& wgcData, CT_DWORD dwGameIndex, CT_WORD wActivityID);

public:
    void QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    void QueryRewardRule(const CT_VOID * pData, CT_DWORD wDataSize);
    void QueryDetail(const CT_VOID * pData, CT_DWORD wDataSize);
    void QueryRewardList(const CT_VOID * pData, CT_DWORD wDataSize);

public:
    inline void UpdateActivityIndexTodb(tagWealthGodComingLastIndex* pLastIndex);
    inline void InsertActivityData(MSG_CS2DB_Insert_WGC_Data* pData);
    inline void UpdateActivetyData(MSG_CS2DB_Update_WGC_Data* pData);
    inline void UpdateActivityUserInfo(MSG_CS2DB_Update_WGC_UserInfo* pUserInfo);
    inline void UpdateActivityAllUserRankInfo(CT_BYTE* pData, CT_DWORD dwSize);
    inline void SendRewardByMail(CT_BYTE* pData, CT_DWORD dwSize);

public:
    inline bool IsSpecialTime(tm &timeNow, int nStartHour, int nStartMin, int nEndHour, int nEndMin);

public:
    void SendDoingWealthGodComingToClient(CT_DWORD dwUserID);
    void BroadcastRealTimeRankInfo(CT_DWORD dwUserID = 0);

private:
    //排名判断标准
    enum enRankJudgeStander
    {
        en_WinLost_Judge = 1,   //输赢判断
        en_Jetton_Judge  = 2,   //下注总额判断
    };
    
    MapWealthGodComingCfg                   m_mapWealthGodComingCfg;                //财神降临的配置
    MapWealthGodComingCfg2                  m_mapWealthGodComingCfg2;               //财神降临的配置2(跟m_mapWealthGodComingCfg一样的配置，不同的map key，牺牲内存换速度)
    MapWealthGodComingReward                m_mapWealthGodComingReward;             //财神降临的奖励配置
    MapWealthGodComingLastIndex             m_mapWealthGodComingLastIndex;          //财神降临的场次
    
    MapDoingWealthGodComing                 m_mapDoingWealthGodComingData;          //正在进行的财神降临活动
    MapDoingWealthGodComingIndex            m_mapDoingWealthGodComingIndex;         //正在进行的财神降临活动的索引
    MapWealthGodComingUserInfo              m_mapDoingWealthGodComingUserInfo;      //正在进行的财神降临活动的用户信息
    MapWealthGodComingUserInfo2             m_mapDoingWealthGodComingUserInfo2;     //正在进行的财神降临活动的用户信息2(只保存了m_mapDoingWealthGodComingUserInfo的指针)
    
    MapHistoryWealthGodComingData           m_mapHistoryWealthGodComingData;        //历史的财神降临的活动信息(汇总信息)
    MapHistoryWealthGodComingUserInfo       m_mapHistoryWealthGodComingUserInfo;    //历史的财神降临活动的排名信息
    CT_BOOL                                 m_bActivityStatusChange;                //活动状态改变
    CT_BOOL                                 m_bDoingActivityStatusChange;           //正在进行的活动
};

#endif //WORKSPACE_ACTIVITYMGR_H