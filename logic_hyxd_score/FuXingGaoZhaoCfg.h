//
// Created by okMan on 2018/11/5.
//

#ifndef _FUXINGGAOZHAOCFG_H
#define _FUXINGGAOZHAOCFG_H

#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "MSG_FXGZ.h"
#include "GameProcess.h"

class CFXGZCfg : public acl::singleton<CFXGZCfg>
{
    friend CGameProcess;
public:
    CFXGZCfg();
    ~CFXGZCfg();

    void LoadCfg();
    inline void LoadBasicInfo(acl::xml1& xml);
    inline void LoadPayTable(acl::xml1& xml);
    inline void LoadPayNotify(acl::xml1& xml);
    inline void LoadCheatCaseCfg(acl::xml1& xml);
    inline void LoadPointCheatCaseCfg(acl::xml1& xml);
    inline void LoadMainGameCaseCfg(acl::xml1& xml);
    inline void LoadFreeGameCaseCfg(acl::xml1& xml);

    bool stringSplit(const std::string& src, const std::string delim, std::vector<std::string>& ret)
    {
        ret.clear();

        size_t last = 0;
        size_t index = src.find_first_of(delim, last);
        while (index != std::string::npos)
        {
            ret.push_back(src.substr(last, index - last));
            last = index + delim.length();
            index = src.find_first_of(delim, last);
        }

        ret.push_back(src.substr(last));

        return true;
    }

private:
    //赔率表
    struct tagPayTable
    {
        CT_BYTE     cbIconId;           //图标ID
        CT_WORD     wFivePayMult;       //五列相同赔率
        CT_WORD     wFourPayMult;       //四列相同赔率
        CT_WORD     wThreePayMult;      //三列相同赔率
    };

    //赔率反馈表
    struct tagPayNotify
    {
        CT_BYTE     cbNotifyId;
        CT_DWORD    dwPayoutMin;
        CT_DWORD    dwPayoutMax;
    };

    //血控点控方案
    struct tagCheatCaseCfg
    {
        CT_LONGLONG llStock;                    //血池值
        CT_CHAR     cCheatType;                 //血控类型
        CT_INT32    iCheatVal;                  //血控数值
        CT_BYTE     cbMainGameCaseId;           //血控使用的游戏返奖方案
    };

    //点控数据
    struct tagPointCheatCaseCfg
    {
        CT_LONGLONG llUserWinScore;             //玩家上桌赢分
        CT_LONGLONG llCheatScore;               //作弊分
        CT_CHAR     cCheatType;                 //点控类型
        CT_INT32    iCheatVal;                  //点控数值
        CT_BYTE     cbMainGameCaseId;           //点控使用的游戏返奖方案
    };

    struct tagCaseStripInfo
    {
        CT_BYTE     cbColumnID;                 //列ID
        CT_BYTE     cbScatCount;                //铜锣出现的最大数量
        CT_WORD     wIconRation[ICON_COUNT];    //0-10种Icon出现的概率
    };

    struct tagMainGameCaseCfg
    {
        CT_BYTE             cbCaseID;                   //方案ID
        CT_DWORD            dwLimitPayMult;             //主游戏限制赔付倍数
        tagCaseStripInfo    caseStripInfo[ICON_COL];    //5列图标数据
    };

    struct tagFreeGameCaseCfg
    {
        CT_DWORD            dwBasicLimitPay;
        CT_DWORD            dwTotalLimitPay;
        CT_DWORD            dwHighlimitPay;
        CT_BYTE             cbFreeRespinOpen;
        tagCaseStripInfo    caseStripInfo[ICON_COL];
    };

    // 旋转最小等待时间（单位：秒）
    CT_WORD                                        m_wSpinInterval;
    //赔率基数
    CT_WORD                                        m_wPayBasicUnit;
    //税收基数
    CT_DWORD                                       m_dwBasicTaxValue;
    //税收比例
    CT_WORD                                        m_wTaxRatio;
    //税收比例2(给小倍场用)
    CT_WORD                                        m_wTaxRatio2;
    //最多没有奖励的次数
    CT_WORD                                        m_wMaxUnluckyCount;

    //赔率表
    tagPayTable                                     m_payTable[ICON_COUNT-1]; //ICON_COUNT-1是因为金锣不赔的
    //赔率反馈表
    std::vector<tagPayNotify>                       m_vecPayNotify;
    //血控方案
    tagCheatCaseCfg                                 m_cheatCaseCfg[ROOM_NUM][CHEAT_CASE_COUNT];
    //点控方案
    tagPointCheatCaseCfg                            m_pointCheatCaseCfg[POINT_CHEAT_CASE_COUNT];
    //主游戏返奖方案
    tagMainGameCaseCfg                              m_mainCaseCfg[PAY_CASE_COUNT];
    //免费游戏返奖方案
    tagFreeGameCaseCfg                              m_freeCaseCfg;
};

#endif //_FUXINGGAOZHAOCFG_H