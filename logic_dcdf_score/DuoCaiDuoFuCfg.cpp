 //
// Created by okMan on 2018/11/5.
//
#include "DuoCaiDuoFuCfg.h"
#include "glog_wrapper.h"

CDCDFCfg::CDCDFCfg()
{
}

CDCDFCfg::~CDCDFCfg()
{

}

void CDCDFCfg::LoadCfg()
{
    acl::string buf;
    if (acl::ifstream::load("./script/DCDF_GAME_CFG.xml", &buf) == false)
    {
        LOG(WARNING) << "load DCDF_GAME_CFG.xml error: " <<  acl::last_serror();
        return ;
    }

    acl::xml1 xml;
    xml.update(buf);

    //加载基本信息
    LOG(WARNING) << "-------------------------------load basic info -----------------------------\n";
    LoadBasicInfo(xml);
    //赔率表
    LOG(WARNING) << "-------------------------------load pay table -----------------------------\n";
    LoadPayTable(xml);
    //赔率反馈表
    LOG(WARNING) << "-------------------------------load pay notify -----------------------------\n";
    LoadPayNotify(xml);
    //免费游戏类型配置
    LOG(WARNING) << "-------------------------------load free game type -----------------------------\n";
    LoadFreeGameTypeCfg(xml);
    //免费游戏类型配置 神秘精选（系统随机选中前4种组合的免费次数和框格大小，再组合成免费奖励）
    LOG(WARNING) << "-------------------------------load free mystery pick -----------------------------\n";
    LoadFreeMysteryPickCfg(xml);
    //押注底分 与 底分总押注
    LOG(WARNING) << "-------------------------------load stake -----------------------------\n";
    LoadStakeCfg(xml);
    //游戏图标配置
    LOG(WARNING) << "-------------------------------load game icon -----------------------------\n";
    LoadGameIconCfg(xml);
    //集福聚宝配置--将池金币增加配置
    LOG(WARNING) << "-------------------------------load bonus jackpot -----------------------------\n";
    LoadBonusJackpotCfg(xml);
    //将池开奖配置
    LOG(WARNING) << "-------------------------------load bonus active -----------------------------\n";
    LoadBonusActiveCfg(xml);
}

void CDCDFCfg::LoadBasicInfo(acl::xml1& xml)
{
    acl::xml_node* pRoot = xml.getFirstElementByTag("Root");
    if (pRoot)
    {
        acl::xml_node* pChild = pRoot->first_child();
        while (pChild)
        {
            const char *pChildTag = pChild->tag_name();
            if (pChildTag == NULL)
            {
                pChild = pRoot->next_child();
                continue;
            }

            if (strcmp("spinInterval", pChildTag) == 0)
            {
                const char* pSpinInterval = pChild->text();
                m_wSpinInterval = (CT_WORD)atoi(pSpinInterval);
                LOG(WARNING) << "spinInterval: " << m_wSpinInterval;
            }
            pChild = pRoot->next_child();
        }
    }
}

void CDCDFCfg::LoadPayTable(acl::xml1& xml)
{
    acl::xml_node* pPayTable = xml.getFirstElementByTag("iconPayTable");
    if (pPayTable)
    {
        acl::xml_node* child = pPayTable->first_child();
        while (child)
        {
            tagPayTable payTable;
            memset(&payTable, 0, sizeof(tagPayTable));
            const acl::xml_attr *attr = child->first_attr();
            while (attr)
            {
                //printf("%s=\"%s\"\r\n", attr->get_name(), attr->get_value());
                if (strcmp("iconID", attr->get_name()) == 0)
                {
                    payTable.cbIconId = (CT_BYTE)atoi(attr->get_value());
                }
                else if (strcmp("five", attr->get_name()) == 0)
                {
                    payTable.wFivePayMult = (CT_WORD)atoi(attr->get_value());
                }
                else if (strcmp("four", attr->get_name()) == 0)
                {
                    payTable.wFourPayMult = (CT_WORD)atoi(attr->get_value());
                }
                else if (strcmp("three", attr->get_name()) == 0)
                {
                    payTable.wThreePayMult = (CT_WORD)atoi(attr->get_value());
                }
                else
                {
                    LOG(ERROR) << "get error icon pay table. attr name: " << attr->get_name();
                    attr = child->next_attr();
                    continue;
                }
                attr = child->next_attr();
            }
            m_payTable[payTable.cbIconId - 1] = payTable;
            LOG(WARNING) << "pay icon id: " << (int) payTable.cbIconId << ", five: " << payTable.wFivePayMult << ", four: " << payTable.wFourPayMult << ", three: " << payTable.wThreePayMult;
            child = pPayTable->next_child();
        }
    }
}


 void CDCDFCfg::LoadPayNotify(acl::xml1& xml)
 {
     acl::xml_node* pPayNotify = xml.getFirstElementByTag("payNotify");
     if (pPayNotify)
     {
         acl::xml_node* child = pPayNotify->first_child();
         while (child)
         {
             tagPayNotify payNotify;
             const acl::xml_attr *attr = child->first_attr();
             while (attr)
             {
                 if (strcmp("id", attr->get_name()) == 0)
                 {
                     payNotify.cbNotifyId = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("payoutMin", attr->get_name()) == 0)
                 {
                     payNotify.dwPayoutMin = (CT_DWORD)atoi(attr->get_value());
                 }
                 else if (strcmp("payOutMax", attr->get_name()) == 0)
                 {
                     payNotify.dwPayoutMax = (CT_DWORD)atoi(attr->get_value());
                 }
                 else if (strcmp("desc", attr->get_name()) == 0)
                 {
                     //这个属性值不读取
                 }
                 else
                 {
                     LOG(ERROR) << "get error pay notify. attr name: " << attr->get_name();
                     attr = child->next_attr();
                     continue;
                 }
                 attr = child->next_attr();
             }
             m_vecPayNotify.push_back(payNotify);
             LOG(WARNING) << "pay notify, id: " << (int)payNotify.cbNotifyId << ", payoutMin: " << payNotify.dwPayoutMin << ", payOutMax: " << payNotify.dwPayoutMax;
             child = pPayNotify->next_child();
         }
     }
 }

 void CDCDFCfg::LoadFreeGameTypeCfg(acl::xml1& xml)
 {
     acl::xml_node* pNode = xml.getFirstElementByTag("freeGameTypeCfg");
     if (pNode)
     {
         m_mFreeTypeCfg.clear();
         acl::xml_node* child = pNode->first_child();
         while (child)
         {
             tagFreeGameTypeInfoCfg freeGameTypeInfoCfg;
             memset(&freeGameTypeInfoCfg, 0, sizeof(freeGameTypeInfoCfg));
             const acl::xml_attr *attr = child->first_attr();
             while (attr)
             {
                 //printf("%s=\"%s\"\r\n", attr->get_name(), attr->get_value());
                 if (strcmp("cbFreeGameType", attr->get_name()) == 0)
                 {
                     freeGameTypeInfoCfg.cbFreeGameType = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("cbRows", attr->get_name()) == 0)
                 {
                     freeGameTypeInfoCfg.cbRows = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("dwFreeGameTimes", attr->get_name()) == 0)
                 {
                     freeGameTypeInfoCfg.dwFreeGameTimes = (CT_DWORD)atoi(attr->get_value());
                 }
                 else
                 {
                     LOG(ERROR) << "get error icon pay table. attr name: " << attr->get_name();
                     attr = child->next_attr();
                     continue;
                 }
                 attr = child->next_attr();
             }
             LOG(WARNING) << "cbFreeGameType: " << (int) freeGameTypeInfoCfg.cbFreeGameType << ", cbRows: " << (int)freeGameTypeInfoCfg.cbRows << ", dwFreeGameTimes: " << freeGameTypeInfoCfg.dwFreeGameTimes;
             m_mFreeTypeCfg.insert(std::make_pair(freeGameTypeInfoCfg.cbFreeGameType, freeGameTypeInfoCfg));
             child = pNode->next_child();
         }
     }
 }

 void CDCDFCfg::LoadFreeMysteryPickCfg(acl::xml1& xml)
 {
     acl::xml_node* pfreeGameCfg = xml.getFirstElementByTag("freeGameMysteryPick");
     if (pfreeGameCfg)
     {
         m_vMysteryPickCfg.clear();
         CT_WORD wTotalRowProba = 0;
         acl::xml_node* pItemRows= pfreeGameCfg->first_child();
         while (pItemRows)
         {
             tagMysteryPickCfg mysteryPickCfg;
             memset(&mysteryPickCfg, 0, sizeof(mysteryPickCfg));
             const char* pRows = pItemRows->attr_value("cbRows");
             if (!pRows)
             {
                 LOG(ERROR) << "LoadFreeMysteryPickCfg, can not find row";
                 break;
             }
             mysteryPickCfg.cbRows = (CT_BYTE)atoi(pRows);
             const char* pProba = pItemRows->attr_value("wProba");
             if (!pProba)
             {
                 LOG(ERROR) << "LoadFreeMysteryPickCfg, can not find Proba";
                 break;
             }
             mysteryPickCfg.wProba = (CT_WORD)atoi(pProba);
             LOG(WARNING) << "reeGameMysteryPick cbRows:" << (int)mysteryPickCfg.cbRows << " wProba: " << (int)mysteryPickCfg.wProba;
             wTotalRowProba += mysteryPickCfg.wProba;
             acl::xml_node* child = pItemRows->first_child();
             CT_FLOAT fTotalTimesProba = 0;
             while (child)
             {
                 tagMysteryPickTimesCfg pickTimesCfg;
                 memset(&pickTimesCfg, 0, sizeof(pickTimesCfg));
                 const acl::xml_attr *attr = child->first_attr();
                 while (attr)
                 {
                     if (strcmp("cbTimes", attr->get_name()) == 0)
                     {
                         pickTimesCfg.cbTimes = (CT_BYTE)atoi(attr->get_value());
                     }
                     else if (strcmp("fProba", attr->get_name()) == 0)
                     {
                         pickTimesCfg.fProba = (CT_FLOAT)atof(attr->get_value());
                         fTotalTimesProba += pickTimesCfg.fProba;
                     }
                     else
                     {
                         LOG(ERROR) << "get error pickTimesCfg case cfg. attr name: " << attr->get_name();
                         attr = child->next_attr();
                         continue;
                     }
                     attr = child->next_attr();
                 }
                 LOG(WARNING) << "cbTimes:" << (int)pickTimesCfg.cbTimes << " fProba: " << pickTimesCfg.fProba;
                 child = pItemRows->next_child();
                 mysteryPickCfg.vMysteryPickTimes.push_back(pickTimesCfg);
             }
             if((CT_WORD)fTotalTimesProba != 100)
             {
                 LOG(ERROR) << "LoadFreeMysteryPickCfg cbTimes proba Sum != 100";
             }

             m_vMysteryPickCfg.push_back(mysteryPickCfg);
             pItemRows = pfreeGameCfg->next_child();
         }
         if(100 != wTotalRowProba)
         {
             LOG(ERROR) << "LoadFreeMysteryPickCfg cbRows proba Sum != 100";
         }
     }
 }

 void CDCDFCfg::LoadStakeCfg(acl::xml1& xml)
 {
     acl::xml_node* pNode = xml.getFirstElementByTag("stakeTable");
     if (pNode)
     {
         m_vStakeCfg.clear();
         acl::xml_node* child = pNode->first_child();
         while (child)
         {
             tagStakeCfg stakeCfg;
             memset(&stakeCfg, 0, sizeof(stakeCfg));
             const acl::xml_attr *attr = child->first_attr();
             while (attr)
             {
                 //printf("%s=\"%s\"\r\n", attr->get_name(), attr->get_value());
                 if (strcmp("betIndex", attr->get_name()) == 0)
                 {
                     stakeCfg.cbBetIndex = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("betMultiplier", attr->get_name()) == 0)
                 {
                     stakeCfg.wBetMultiplier = (CT_WORD)atoi(attr->get_value());
                 }
                 else if (strcmp("betBaseStake", attr->get_name()) == 0)
                 {
                     stakeCfg.wBetBaseStake = (CT_WORD)atoi(attr->get_value());
                 }
                 else if (strcmp("betTotalStake", attr->get_name()) == 0)
                 {
                     stakeCfg.llBetTotalStake = atoll(attr->get_value());
                 }
                 else
                 {
                     LOG(ERROR) << "get error icon tagStakeCfg. attr name: " << attr->get_name();
                     attr = child->next_attr();
                     continue;
                 }
                 attr = child->next_attr();
             }
             LOG(WARNING) << "betIndex: " << (int) stakeCfg.cbBetIndex << ", betMultiplier: " << (int)stakeCfg.wBetMultiplier << ", betBaseStake: " << stakeCfg.wBetBaseStake << ", betTotalStake:" << stakeCfg.llBetTotalStake;
            m_vStakeCfg.push_back(stakeCfg);
             child = pNode->next_child();
         }
     }
 }

 void CDCDFCfg::LoadGameIconCfg(acl::xml1& xml)
 {
     acl::xml_node* pGameIconCfg = xml.getFirstElementByTag("gameIconCfg");
     if (pGameIconCfg)
     {
         m_mGameIconCfg.clear();
         acl::xml_node* pGameItem= pGameIconCfg->first_child();
         while (pGameItem)
         {
             int iReelIndex = INVALID_CHAIR;
             const char* pReelIndex = pGameItem->attr_value("reelIndex");
             if (!pReelIndex)
             {
                 LOG(ERROR) << "LoadGameIconCfg, can not find reelIndex";
                 break;
             }
             iReelIndex = (CT_BYTE)atoi(pReelIndex);
             acl::xml_node* child = pGameItem->first_child();
             if(iReelIndex < 0 || iReelIndex > 4)
             {
                 LOG(ERROR) << "LoadGameIconCfg, reelIndex error:" << iReelIndex;
                 break;
             }
             std::vector<tagGameIconCfg> & vGameIcon = m_mGameIconCfg[iReelIndex];
             while (child)
             {
                 tagGameIconCfg gameIconCfg;
                 memset(&gameIconCfg, 0, sizeof(gameIconCfg));
                 const acl::xml_attr *attr = child->first_attr();
                 std::vector<std::string> vecScatRatio;
                 while (attr)
                 {
                     if (strcmp("StripColumnID", attr->get_name()) == 0)
                     {
                         gameIconCfg.cbStripColumnID = (CT_BYTE)atoi(attr->get_value());
                     }
                     else if (strcmp("indexCounts", attr->get_name()) == 0)
                     {
                         gameIconCfg.wIndexCounts = (CT_WORD)atoi(attr->get_value());
                     }
                     else if(strcmp("iconReelInfo", attr->get_name()) == 0)
                     {
                         vecScatRatio.clear();
                         stringSplit(attr->get_value(), ";", vecScatRatio);
                         for (CT_DWORD i = 0; i < vecScatRatio.size(); ++i)
                         {
                             gameIconCfg.vIconIDInfo.push_back((CT_BYTE)atoi(vecScatRatio[i].c_str()));
                         }
                     }
                     else
                     {
                         LOG(ERROR) << "get error LoadGameIconCfg case cfg. attr name: " << attr->get_name();
                         attr = child->next_attr();
                         continue;
                     }
                     attr = child->next_attr();
                 }

                 child = pGameItem->next_child();
                 LOG(WARNING) << "iReelIndex: " << iReelIndex << ", StripColumnID: " << (int)gameIconCfg.cbStripColumnID << ", IndexCounts: " << (int)gameIconCfg.wIndexCounts << ", iconReelInfo-Len:" << vecScatRatio.size();
                 vGameIcon.push_back(gameIconCfg);
             }
             pGameItem = pGameIconCfg->next_child();
         }
     }
 }

 void CDCDFCfg::LoadBonusJackpotCfg(acl::xml1& xml)
 {
     acl::xml_node* pNode = xml.getFirstElementByTag("bonusJackpot");
     if (pNode)
     {
         const char* pBaseStake = pNode->attr_value("baseStake");
         if (!pBaseStake)
         {
             LOG(ERROR) << "LoadBonusJackpotCfg, can not find baseStake";
             return;
         }
         m_BonusJackpotCfg.cbBaseStake = (CT_BYTE) atoi(pBaseStake);
         const char* pBaseTotalStake = pNode->attr_value("baseTotalStake");
         if (!pBaseTotalStake)
         {
             LOG(ERROR) << "LoadBonusJackpotCfg, can not find pBaseTotalStake";
             return;
         }
         m_BonusJackpotCfg.wBaseTotalStake = (CT_BYTE) atoi(pBaseTotalStake);
         m_BonusJackpotCfg.vBonusInfo.clear();
         acl::xml_node* child = pNode->first_child();
         while (child)
         {
             tagBonusJackpotTable jackpotTable;
             memset(&jackpotTable, 0, sizeof(jackpotTable));
             const acl::xml_attr *attr = child->first_attr();
             while (attr)
             {
                 //printf("%s=\"%s\"\r\n", attr->get_name(), attr->get_value());
                 if (strcmp("jackpotIndex", attr->get_name()) == 0)
                 {
                     jackpotTable.cbIndex = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("jackpotName", attr->get_name()) == 0)
                 {
                     strncmp(jackpotTable.szName, attr->get_value(), sizeof(jackpotTable.szName));
                 }
                 else if (strcmp("baseValue", attr->get_name()) == 0)
                 {
                     jackpotTable.llBaseValue = atoll(attr->get_value());
                 }
                 else if (strcmp("plusValue", attr->get_name()) == 0)
                 {
                     jackpotTable.fPlusValue = atof(attr->get_value());
                 }
                 else
                 {
                     LOG(ERROR) << "get error icon LoadBonusJackpotCfg. attr name: " << attr->get_name();
                     attr = child->next_attr();
                     continue;
                 }
                 attr = child->next_attr();
             }
             LOG(WARNING) << "jackpotIndex: " << (int) jackpotTable.cbIndex << ", jackpotName: " << jackpotTable.szName << ", baseValue: " << jackpotTable.llBaseValue << ", plusValue:" << jackpotTable.fPlusValue;
             m_BonusJackpotCfg.vBonusInfo.push_back(jackpotTable);
             child = pNode->next_child();
         }
     }
 }

 void CDCDFCfg::LoadBonusActiveCfg(acl::xml1& xml)
 {
     acl::xml_node* pNode = xml.getFirstElementByTag("bonusActive");
     if (pNode)
     {
         const char* pCollectCounts = pNode->attr_value("collectCounts");
         if (!pCollectCounts)
         {
             LOG(ERROR) << "LoadBonusActiveCfg, can not find baseStake";
             return;
         }
         m_BonusActiveCfg.dwCollectCounts = (CT_DWORD)atol(pCollectCounts);
         const char* pIsBless = pNode->attr_value("isBless");
         if (!pIsBless)
         {
             LOG(ERROR) << "LoadBonusActiveCfg, can not find pIsBless";
             return;
         }
         m_BonusActiveCfg.bIsBless = (CT_BOOL)atoi(pIsBless);
         const char* pTriggerProb = pNode->attr_value("triggerProb");
         if (!pTriggerProb)
         {
             LOG(ERROR) << "LoadBonusActiveCfg, can not find pBaseTotalStake";
             return;
         }
         m_BonusActiveCfg.fTriggerProb = atof(pTriggerProb);
         m_BonusActiveCfg.vBonusInfo.clear();
         acl::xml_node* child = pNode->first_child();
         while (child)
         {
             tagBonusActiveTable activeTable;
             memset(&activeTable, 0, sizeof(activeTable));
             const acl::xml_attr *attr = child->first_attr();
             while (attr)
             {
                 //printf("%s=\"%s\"\r\n", attr->get_name(), attr->get_value());
                 if (strcmp("jackpotIndex", attr->get_name()) == 0)
                 {
                     activeTable.cbJackpotIndex = (CT_BYTE)atoi(attr->get_value());
                 }
                 else if (strcmp("triggerWeight", attr->get_name()) == 0)
                 {
                     activeTable.wTriggerWeight = atoi(attr->get_value());
                 }
                 else
                 {
                     LOG(ERROR) << "get error icon LoadBonusActiveCfg. attr name: " << attr->get_name();
                     attr = child->next_attr();
                     continue;
                 }
                 attr = child->next_attr();
             }
             LOG(WARNING) << "jackpotIndex: " << (int) activeTable.cbJackpotIndex << ", triggerWeight: " << activeTable.wTriggerWeight;
             m_BonusActiveCfg.vBonusInfo.push_back(activeTable);
             child = pNode->next_child();
         }
     }
 }