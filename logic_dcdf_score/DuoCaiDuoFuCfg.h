//
// Created by okMan on 2018/11/5.
//

#ifndef _DUOCAIDUOFUCFG_H
#define _DUOCAIDUOFUCFG_H

#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "MSG_dcdf.h"
#include "GameProcess.h"

class CDCDFCfg : public acl::singleton<CDCDFCfg>
{
    friend CGameProcess;
public:
    CDCDFCfg();
    ~CDCDFCfg();

    void LoadCfg();
    inline void LoadBasicInfo(acl::xml1& xml);
    inline void LoadPayTable(acl::xml1& xml);
    inline void LoadPayNotify(acl::xml1& xml);
    inline void LoadFreeGameTypeCfg(acl::xml1& xml);
    inline void LoadFreeMysteryPickCfg(acl::xml1& xml);
    inline void LoadStakeCfg(acl::xml1& xml);
    inline void LoadGameIconCfg(acl::xml1& xml);
    inline void LoadBonusJackpotCfg(acl::xml1& xml);
    inline void LoadBonusActiveCfg(acl::xml1& xml);

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

public:
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

    struct tagFreeGameTypeInfoCfg
    {
        CT_BYTE            cbFreeGameType;          //类型
        CT_BYTE            cbRows;                  //行数
        CT_DWORD           dwFreeGameTimes;         //免费游戏次数
    };

    //免费游戏类型5 神秘精选配置---次数概率
    struct tagMysteryPickTimesCfg
    {
        CT_BYTE            cbTimes;                 //次数
        CT_FLOAT           fProba;                  //概率
    };
    //免费游戏类型5 神秘精选配置
    struct tagMysteryPickCfg
    {
        CT_BYTE            cbRows;                  //行数
        CT_WORD            wProba;                  //概率
        std::vector<tagMysteryPickTimesCfg>  vMysteryPickTimes;
    };

    //押注底分 与 底分总押注
    struct tagStakeCfg
    {
        CT_BYTE cbBetIndex;                         //押注索引
        CT_WORD wBetMultiplier;                     //押注底分倍数
        CT_WORD wBetBaseStake;                      //押注底分
        CT_LONGLONG llBetTotalStake;                //总押注
    };

    //游戏图标配置
    struct tagGameIconCfg
    {
        CT_BYTE             cbStripColumnID;        //列
        CT_WORD             wIndexCounts;           //数量随机位置最大值
        std::vector<CT_BYTE> vIconIDInfo;           //图标ID
    };
    //集福聚宝配置
    struct tagBonusJackpotTable
    {
        CT_BYTE             cbIndex;                     //索引
        CT_CHAR             szName[NICKNAME_LEN];        //单次总赌注
        CT_LONGLONG         llBaseValue;                 //基础奖金
        CT_FLOAT            fPlusValue;                  //押注88时彩金增益值
    };
    //集福聚宝配置--将池金币增加配置
    struct tagBonusJackpotCfg
    {
        CT_BYTE             cbBaseStake;                //基础底分赌注
        CT_WORD             wBaseTotalStake;            //单次总赌注
        std::vector<tagBonusJackpotTable> vBonusInfo;   //聚宝鹏配置表
    };

    struct tagBonusActiveTable
    {
        CT_BYTE cbJackpotIndex;                         //将池索引
        CT_WORD wTriggerWeight;                         //是指各彩金类型开奖的万分比概率
    };
    //将池开奖配置
    struct tagBonusActiveCfg
    {
        CT_DWORD            dwCollectCounts;            //是指需要集福的个数,即单档押注下所有玩家旋转结果至少需要出现福的次数;
        CT_BOOL             bIsBless;                   //是否需要福才能开奖
        CT_FLOAT            fTriggerProb;               //是指集福次数达标后，以后每次旋转有概率开启聚宝盆的概率（万分比
        std::vector<tagBonusActiveTable> vBonusInfo;    //具体开那个将池的奖金给玩家的配置
    };

    // 旋转最小等待时间（单位：秒）
    CT_WORD                                        m_wSpinInterval;
    //赔率表
    tagPayTable                                     m_payTable[ICON_COUNT-1]; //ICON_COUNT-1是因为金锣不赔的
    //赔率反馈表
    std::vector<tagPayNotify>                       m_vecPayNotify;
    //免费游戏类型及行数及次数配置
    std::map< CT_BYTE ,tagFreeGameTypeInfoCfg>      m_mFreeTypeCfg;
    //免费游戏类型5 神秘精选配置
    std::vector<tagMysteryPickCfg>                  m_vMysteryPickCfg;
    //押注底分 与 底分总押注
    std::vector<tagStakeCfg>                        m_vStakeCfg;
    //游戏图标配置
    std::map<int, std::vector<tagGameIconCfg>>      m_mGameIconCfg;         //int 表示配置文件的reelIndex,0表示主游戏，1表示免费游戏类型1...
    //集福聚宝配置--将池金币增加配置
    tagBonusJackpotCfg                              m_BonusJackpotCfg;
    //将池开奖配置
    tagBonusActiveCfg                               m_BonusActiveCfg;
};

#endif //_FUXINGGAOZHAOCFG_H