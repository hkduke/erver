//
// Created by luwei on 2018/10/20.
//

#ifndef WORKSPACE_DUOBAO_H
#define WORKSPACE_DUOBAO_H

#include "acl_cpp/lib_acl.hpp"
#include <CTType.h>
#include <GlobalStruct.h>
#include <map>
#include <CMD_Plaza.h>
#include "UserMgr.h"



class CYYDuoBaoMgr : public acl::singleton<CYYDuoBaoMgr>
{
public:
    CYYDuoBaoMgr();
    ~CYYDuoBaoMgr();

public:
    void InsertDuoBaoCfg(tagDuoBaoCfg* pDuoBaoCfg);
    void UpdateDuoBaoCfg(tagDuoBaoCfg* pDuoBaoCfg);

    void InsertDoingDuoBaoData(tagDuoBaoData* pDuoBaoData);
    void InsertDoingDuoBaoUser(tagDuoBaoUserInfo* pDuoBaoUser);

    void InsertHistoryDuoBaoData(tagDuoBaoData* pDuoBaoData);
    void InsertHistoryDuoBaoUser(tagDuoBaoUserInfo* pDuoBaoUser);

    void SetLoadDataFinish(bool bLoadData);
    void SetSystemState(CT_BYTE cbState);

    void CheckDuoBaoStartEnd();

public:
    void DuoBaoMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);

private:
    //协议消息
    inline void QueryMainInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void QuerySelfLuckyNum(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void QuerySelfHistory(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void QueryHistoryLuckyUser(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void BuyLuckyNum(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void CloseUi(const CT_VOID * pData, CT_DWORD wDataSize);
    inline void QueryRewardRecord(const CT_VOID * pData, CT_DWORD wDataSize);

    //更新数据库
    inline void UpdateDataToDB(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr);
    inline void InsertUserInfoToDB(std::unique_ptr<tagDuoBaoUserInfo>& duobaoUserPtr);

private:
    inline void FlowOneDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr);
    inline void OpenOneDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr);
    inline void ClearOpenDuoBao(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr);
    inline void UpdateDoingDuoBaoState(std::unique_ptr<tagDuoBaoData>& duobaoDataPtr, CT_DWORD dwUserID);
    inline void UpdateAllDuoBaoState();

private:
    //一元夺宝的配置
    //<--id, --tagDuoBaoCfg>
    typedef std::map<CT_WORD, tagDuoBaoCfg> MapDuoBaoCfg;
    typedef std::vector<tagDuoBaoCfg*>  vecDuoBaoCfg;

    //一元夺宝的本期信息
    typedef std::map<CT_WORD, std::unique_ptr<tagDuoBaoData>> MapDoingDuoBaoData;

    //一元夺宝的本期用户购买信息(一个人购买多次会有些少内存浪费)
    //<--Id*DUOBAO_INDEX+Index, <--userId, std::vector<userInfo>>>
    typedef std::vector<std::unique_ptr<tagDuoBaoUserInfo>> VecDuoBaoUserInfo;
    typedef std::map<CT_DWORD, VecDuoBaoUserInfo> MapDuoBaoUserDetail;
    typedef std::map<CT_DWORD,  MapDuoBaoUserDetail> MapDuoBaoUser;
    //冗余数据
    //<--Id*DUOBAO_INDEX+Index, <--luckyNum, --UserId>>
    typedef std::map<CT_DWORD, CT_DWORD> MapLuckyNum;
    typedef std::map<CT_DWORD, MapLuckyNum> MapLuckyNum2User;

    //一元夺宝的往期记录(当前进行的记录也有里面)
    typedef std::list<std::unique_ptr<tagDuoBaoData>> ListHistoryDuoBaoData;
    typedef std::map<CT_WORD, ListHistoryDuoBaoData> MapHistoryDuoBaoData;

    //打开夺宝界面的用户
    typedef std::set<CT_DWORD> SetOpenUiUser;

    #define DUOBAO_INDEX 1000000000
    #define DAY_SECOND 86400
    #define LUCKY_NUM  100000

    MapDuoBaoCfg                m_mapDuoBaoCfg;         //夺宝配置
    vecDuoBaoCfg                m_vecDuoBaoCfg;         //夺宝配置

    MapDoingDuoBaoData          m_mapDoingData;         //正在进行的夺宝信息
    MapDuoBaoUser               m_mapDoingUser;         //参与正在进行的夺宝用户
    MapLuckyNum2User            m_mapDoingLuckNum;      //正在进行的夺宝码

    MapHistoryDuoBaoData        m_mapHistoryData;       //一个月内的历史夺宝信息
    MapDuoBaoUser               m_mapHistoryUser;       //一个月内的夺宝用户

    SetOpenUiUser               m_setOpenUiUser;        //打开界面的玩家

    bool                        m_bLoadData;            //是否完成信息加载
    CT_BYTE                     m_cbSysState;           //系统状态
};

#endif //WORKSPACE_DUOBAO_H