 //
// Created by okMan on 2018/11/5.
//
#include "FuXingGaoZhaoCfg.h"
#include "glog_wrapper.h"

CFXGZCfg::CFXGZCfg()
{
}

CFXGZCfg::~CFXGZCfg()
{

}

void CFXGZCfg::LoadCfg()
{
    acl::string buf;
    if (acl::ifstream::load("./script/FXGZ_GAME_CFG.xml", &buf) == false)
    {
        LOG(WARNING) << "load FXGZ_GAME_CFG.xml error: " <<  acl::last_serror();
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
    //血控方案
    LOG(WARNING) << "-------------------------------load cheat case -----------------------------\n";
    LoadCheatCaseCfg(xml);
    //点控方案
    LOG(WARNING) << "-------------------------------load point cheat case -----------------------------\n";
    LoadPointCheatCaseCfg(xml);
    //主游戏控制方案
    LOG(WARNING) << "-------------------------------load main game case -----------------------------\n";
    LoadMainGameCaseCfg(xml);
    //免费游戏控制方案
    LOG(WARNING) << "-------------------------------load free game case -----------------------------\n";
    LoadFreeGameCaseCfg(xml);
}

void CFXGZCfg::LoadBasicInfo(acl::xml1& xml)
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
            else if (strcmp("payBasicUnit", pChildTag) == 0)
            {
                const char* pPayBasicUnit = pChild->text();
                m_wPayBasicUnit = (CT_WORD)atoi(pPayBasicUnit);
                LOG(WARNING) << "payBasicUnit: " << m_wPayBasicUnit;
            }
            else if (strcmp("taxBasicUnit", pChildTag) == 0)
            {
                const char* pTaxBasicUnit = pChild->text();
                m_dwBasicTaxValue = (CT_DWORD)atoi(pTaxBasicUnit);
                LOG(WARNING) << "taxBasicUnit: " << m_dwBasicTaxValue;
            }
            else if (strcmp("taxRatio", pChildTag) == 0)
            {
                const char* pTaxRatio = pChild->text();
                m_wTaxRatio = (CT_WORD)atoi(pTaxRatio);
                LOG(WARNING) << "taxRatio: " << m_wTaxRatio;
            }
            else if (strcmp("taxRatio2", pChildTag) == 0)
            {
                const char* pTaxRatio2 = pChild->text();
                m_wTaxRatio2 = (CT_WORD)atoi(pTaxRatio2);
                LOG(WARNING) << "taxRatio2: " << m_wTaxRatio2;
            }
            else if (strcmp("maxUnluckyCount", pChildTag) == 0)
            {
                const char* pMaxUnluckyCount = pChild->text();
                m_wMaxUnluckyCount = (CT_WORD)atoi(pMaxUnluckyCount);
                LOG(WARNING) << "maxUnluckyCount: " << m_wMaxUnluckyCount;
            }

            pChild = pRoot->next_child();
        }
    }
}

void CFXGZCfg::LoadPayTable(acl::xml1& xml)
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

void CFXGZCfg::LoadPayNotify(acl::xml1& xml)
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

void CFXGZCfg::LoadCheatCaseCfg(acl::xml1& xml)
{
    acl::xml_node* pCheatCfg = xml.getFirstElementByTag("cheatCaseCFG");
    if (pCheatCfg)
    {
        acl::xml_node* pRoomCheatCfgChild= pCheatCfg->first_child();
        while (pRoomCheatCfgChild)
        {
            const char* pRoomId = pRoomCheatCfgChild->attr_value("roomId");
            if (!pRoomId)
            {
                LOG(ERROR) << "LoadCheatCaseCfg, can not find room id";
                break;
            }

            acl::xml_node* child = pRoomCheatCfgChild->first_child();
            int nCount = 0;
            while (child)
            {
                tagCheatCaseCfg cheatCaseCfg;
                memset(&cheatCaseCfg, 0, sizeof(cheatCaseCfg));
                const acl::xml_attr *attr = child->first_attr();
                while (attr)
                {
                    if (strcmp("cheatType", attr->get_name()) == 0)
                    {
                        cheatCaseCfg.cCheatType = (CT_CHAR)atoi(attr->get_value());
                    }
                    else if (strcmp("stock", attr->get_name()) == 0)
                    {
                        cheatCaseCfg.llStock = atoll(attr->get_value()) * TO_LL;
                    }
                    else if (strcmp("cheatVal", attr->get_name()) == 0)
                    {
                        cheatCaseCfg.iCheatVal = atoi(attr->get_value());
                    }
                    else if (strcmp("mainGameCaseID", attr->get_name()) == 0)
                    {
                        cheatCaseCfg.cbMainGameCaseId = (CT_BYTE)atoi(attr->get_value());
                    }
                    else
                    {
                        LOG(ERROR) << "get error cheat case cfg. attr name: " << attr->get_name();
                        attr = child->next_attr();
                        continue;
                    }
                    attr = child->next_attr();
                }

                int nRoomId = atoi(pRoomId);
                child = pRoomCheatCfgChild->next_child();
                m_cheatCaseCfg[nRoomId-1][nCount++] = cheatCaseCfg;
                LOG(WARNING) << "room " << nRoomId << " main game case, cheatType: " << (int)cheatCaseCfg.cCheatType  << ", stock: " << cheatCaseCfg.llStock/100
                             << ", cheatVal: " << cheatCaseCfg.iCheatVal << ", mainGameCaseID: " << (int)cheatCaseCfg.cbMainGameCaseId;
                if (nCount >= CHEAT_CASE_COUNT)
                    break;
            }
            pRoomCheatCfgChild = pCheatCfg->next_child();
        }
    }
}

void CFXGZCfg::LoadPointCheatCaseCfg(acl::xml1& xml)
{
    acl::xml_node* pCheatCfg = xml.getFirstElementByTag("pointCheatCaseCFG");
    if (pCheatCfg)
    {
        acl::xml_node* child = pCheatCfg->first_child();
        int nCount = 0;
        while (child)
        {
            tagPointCheatCaseCfg cheatCaseCfg;
            const acl::xml_attr *attr = child->first_attr();
            while (attr)
            {
                if (strcmp("cheatType", attr->get_name()) == 0)
                {
                    cheatCaseCfg.cCheatType = (CT_CHAR)atoi(attr->get_value());
                }
                else if (strcmp("userWinScore", attr->get_name()) == 0)
                {
                    cheatCaseCfg.llUserWinScore = atoll(attr->get_value()) * TO_LL;
                }
                else if (strcmp("cheatScore", attr->get_name()) == 0)
                {
                    cheatCaseCfg.llCheatScore = atoll(attr->get_value()) * TO_LL;
                }
                else if (strcmp("cheatVal", attr->get_name()) == 0)
                {
                    cheatCaseCfg.iCheatVal = atoi(attr->get_value());
                }
                else if (strcmp("mainGameCaseID", attr->get_name()) == 0)
                {
                    cheatCaseCfg.cbMainGameCaseId = (CT_BYTE)atoi(attr->get_value());
                }
                else
                {
                    LOG(ERROR) << "get error point cheat case cfg. attr name: " << attr->get_name();
                    attr = child->next_attr();
                    continue;
                }
                attr = child->next_attr();
            }
            m_pointCheatCaseCfg[nCount++] = cheatCaseCfg;
            LOG(WARNING) << "point case, cheatType: " << (int)cheatCaseCfg.cCheatType  << ", userWinScore: " << cheatCaseCfg.llUserWinScore/100
                << ", cheatScore: " << cheatCaseCfg.llCheatScore/100 << ", cheatVal: " << cheatCaseCfg.iCheatVal << ", mainGameCaseID: " << (int)cheatCaseCfg.cbMainGameCaseId;
            if (nCount >= POINT_CHEAT_CASE_COUNT)
                break;
            child = pCheatCfg->next_child();
        }
    }
}

#include <iostream>
void CFXGZCfg::LoadMainGameCaseCfg(acl::xml1& xml)
{
    acl::xml_node* pGameRoomCfg = xml.getFirstElementByTag("gameRoomCFG");
    if (pGameRoomCfg)
    {
        acl::xml_node* pGameRoom = pGameRoomCfg->first_child();
        if (pGameRoom)
        {
            const char* pTagName =  pGameRoom->tag_name();
            if (strcmp("gameRoom", pTagName) == 0)
            {
                acl::xml_node* pMainReturnCaseCfg = pGameRoom->first_child();
                if (pMainReturnCaseCfg)
                {
                    pTagName = pMainReturnCaseCfg->tag_name();
                    if (strcmp("mainReturnCaseCFG", pTagName) != 0)
                    {
                        LOG(ERROR) << "can not find mainReturnCaseCFG tag.";
                        return;
                    }

                    acl::xml_node* pMainGameCase = pMainReturnCaseCfg->first_child();
                    if (pMainGameCase)
                    {
                        while (pMainGameCase)
                        {
                            pTagName = pMainGameCase->tag_name();
                            if (strcmp("mainGameCase", pTagName) == 0)
                            {
                                const char* pMainGameCaseId = pMainGameCase->attr_value("caseID");
                                auto cbCaseID = 0;
                                if (pMainGameCaseId)
                                {
                                    LOG(WARNING) << "MainGameCaseId: " << pMainGameCaseId;
                                    cbCaseID = (CT_BYTE)atoi(pMainGameCaseId);
                                }

                                tagMainGameCaseCfg& mainGameCaseCfg = m_mainCaseCfg[cbCaseID - 1];
                                mainGameCaseCfg.cbCaseID = cbCaseID;

                                acl::xml_node* pMainGameCaseChild = pMainGameCase->first_child();
                                while (pMainGameCaseChild)
                                {
                                    const char *pMainGameCaseChildTag = pMainGameCaseChild->tag_name();
                                    if (pMainGameCaseChildTag == NULL)
                                        return;

                                    if (strcmp("limitPayMult", pMainGameCaseChildTag) == 0)
                                    {
                                        const char* pLimitPayMult = pMainGameCaseChild->text();
                                        LOG(WARNING) << "limitPayMult: " << pLimitPayMult;
                                        mainGameCaseCfg.dwLimitPayMult = atoi(pLimitPayMult);
                                    }
                                    else if (strcmp("caseStripInfo", pMainGameCaseChildTag) == 0)
                                    {
                                       acl::xml_node* pCaseStripInfoChild = pMainGameCaseChild->first_child();
                                        int nChildCount = 0;
                                        while (pCaseStripInfoChild)
                                       {
                                           const acl::xml_attr *attr = pCaseStripInfoChild->first_attr();
                                           while (attr)
                                           {
                                                if (strcmp("stripColumnID", attr->get_name()) == 0)
                                                {
                                                    LOG(WARNING) << "stripColumnID: " << attr->get_value();
                                                    mainGameCaseCfg.caseStripInfo[nChildCount].cbColumnID = (CT_BYTE)atoi(attr->get_value());
                                                    if (mainGameCaseCfg.caseStripInfo[nChildCount].cbColumnID != (nChildCount+1))
                                                    {
                                                        LOG(WARNING) << "load column id not equal 10";
                                                        exit(0);
                                                    }
                                                }
                                                else if (strcmp("scatCounts", attr->get_name()) == 0)
                                                {
                                                    LOG(WARNING) << "scatCounts: " << attr->get_value();
                                                    mainGameCaseCfg.caseStripInfo[nChildCount].cbScatCount = (CT_BYTE)atoi(attr->get_value());
                                                }
                                                else if (strcmp("scatRatio", attr->get_name()) == 0)
                                                {
                                                    LOG(WARNING) << "scatRatio: " << attr->get_value();
                                                    std::string scatRatio = attr->get_value();
                                                    std::vector<std::string> vecScatRatio;
                                                    stringSplit(scatRatio, "_", vecScatRatio);
                                                    if (vecScatRatio.size() != 2)
                                                    {
                                                        attr = pCaseStripInfoChild->next_attr();
                                                        continue;
                                                    }
                                                    mainGameCaseCfg.caseStripInfo[nChildCount].wIconRation[0] = (CT_WORD)atoi(vecScatRatio[1].c_str());
                                                }
                                                else if (strcmp("iconWeiAreas", attr->get_name()) == 0)
                                                {
                                                    LOG(WARNING) << "iconWeiAreas: " << attr->get_value();
                                                    std::string iconWeiAreas = attr->get_value();
                                                    std::vector<std::string> vecIconWeiAreas;
                                                    stringSplit(iconWeiAreas, ";", vecIconWeiAreas);
                                                    if (vecIconWeiAreas.size() != 10)
                                                    {
                                                        exit(0);
                                                    }

                                                    for (auto& it : vecIconWeiAreas)
                                                    {
                                                        std::string& strIconWeiAreas = it;
                                                        auto pos = strIconWeiAreas.find_first_of("_");
                                                        auto iconIndex = 0;
                                                        auto iconRatio = 0;
                                                        if (pos == std::string::npos)
                                                        {
                                                            exit(0);
                                                        }
                                                        iconIndex = atoi(strIconWeiAreas.substr(0, pos).c_str());
                                                        iconRatio = atoi(strIconWeiAreas.substr(pos+1).c_str());
                                                        mainGameCaseCfg.caseStripInfo[nChildCount].wIconRation[iconIndex] = iconRatio;
                                                        std::cout << "icon" << iconIndex  << " ration: " << mainGameCaseCfg.caseStripInfo[nChildCount].wIconRation[iconIndex] << " ";
                                                    }
                                                    std::cout << std::endl;
                                                }
                                                else
                                                {
                                                    LOG(WARNING) << "get error caseStripInfo attr. attr name: " << attr->get_name();
                                                    attr = pCaseStripInfoChild->next_attr();
                                                    continue;
                                                }
                                                attr = pCaseStripInfoChild->next_attr();
                                            }
                                           ++nChildCount;
                                            pCaseStripInfoChild = pMainGameCaseChild->next_child();
                                       }
                                    }
                                    else
                                    {
                                        LOG(ERROR) << "can not find caseStripInfo tag. tag name: " << pMainGameCaseChildTag;
                                        pMainGameCaseChild = pMainGameCase->next_child();
                                        continue;
                                    }

                                    pMainGameCaseChild = pMainGameCase->next_child();
                                }
                            }
                            //循环多个MainGameCase
                            pMainGameCase = pMainReturnCaseCfg->next_child();
                        }
                    }
                }
            }
        }
    }
}

void CFXGZCfg::LoadFreeGameCaseCfg(acl::xml1& xml)
{
    acl::xml_node* pGameRoomCfg = xml.getFirstElementByTag("gameRoomCFG");
    if (pGameRoomCfg)
    {
        acl::xml_node* pGameRoom = pGameRoomCfg->first_child();
        if (pGameRoom)
        {
            const char* pTagName =  pGameRoom->tag_name();
            if (strcmp("gameRoom", pTagName) == 0)
            {
                acl::xml_node* pFreeReturnCaseCFG = pGameRoom->first_child();
                while (pFreeReturnCaseCFG)
                {
                    pTagName = pFreeReturnCaseCFG->tag_name();
                    if (strcmp("freeReturnCaseCFG", pTagName) != 0)
                    {
                        pFreeReturnCaseCFG= pGameRoom->next_child();
                        continue;
                    }

                    //找到freeReturnCaseCFG就跳出循环
                    acl::xml_node* pFreeReturnCaseCFGChild = pFreeReturnCaseCFG->first_child();
                    while (pFreeReturnCaseCFGChild)
                    {
                        const char *pFreeReturnCaseCFGChildTag = pFreeReturnCaseCFGChild->tag_name();
                        if (pFreeReturnCaseCFGChildTag == NULL)
                        {
                            pFreeReturnCaseCFGChild = pFreeReturnCaseCFG->first_child();
                            continue;
                        }

                        if (strcmp("basicLimitPay", pFreeReturnCaseCFGChildTag) == 0)
                        {
                            const char* pBasicLimitPay = pFreeReturnCaseCFGChild->text();
                            LOG(WARNING) << "basicLimitPay: " << pBasicLimitPay;
                            m_freeCaseCfg.dwBasicLimitPay = atoi(pBasicLimitPay);
                        }
                        else if (strcmp("totalLimitPay", pFreeReturnCaseCFGChildTag) == 0)
                        {
                            const char* pTotalLimitPay = pFreeReturnCaseCFGChild->text();
                            LOG(WARNING) << "totalLimitPay: " << pTotalLimitPay;
                            m_freeCaseCfg.dwTotalLimitPay = atoi(pTotalLimitPay);
                        }
                        else if (strcmp("highLimitPay", pFreeReturnCaseCFGChildTag) == 0)
                        {
                            const char* pHighLimitPay = pFreeReturnCaseCFGChild->text();
                            LOG(WARNING) << "highLimitPay: " << pHighLimitPay;
                            m_freeCaseCfg.dwHighlimitPay = atoi(pHighLimitPay);
                        }
                        else if (strcmp("freeRespinOpen", pFreeReturnCaseCFGChildTag) == 0)
                        {
                            const char* pFreeRespinOpen = pFreeReturnCaseCFGChild->text();
                            LOG(WARNING) << "freeRespinOpen: " << pFreeRespinOpen;
                            acl::string strFreeRespinOpen(pFreeRespinOpen);
                            strFreeRespinOpen.upper();
                            if (strFreeRespinOpen == "false")
                                m_freeCaseCfg.cbFreeRespinOpen = 0;
                            else
                                m_freeCaseCfg.cbFreeRespinOpen = 1;
                        }
                        else if (strcmp("freeStripInfo", pFreeReturnCaseCFGChildTag) == 0)
                        {
                            acl::xml_node* pCaseStripInfoChild = pFreeReturnCaseCFGChild->first_child();
                            int nChildCount = 0;
                            while (pCaseStripInfoChild)
                            {
                                const acl::xml_attr *attr = pCaseStripInfoChild->first_attr();
                                while (attr)
                                {
                                    if (strcmp("stripColumnID", attr->get_name()) == 0)
                                    {
                                        LOG(WARNING) << "stripColumnID: " << attr->get_value();
                                        m_freeCaseCfg.caseStripInfo[nChildCount].cbColumnID = (CT_BYTE)atoi(attr->get_value());
                                        if (m_freeCaseCfg.caseStripInfo[nChildCount].cbColumnID != (nChildCount+1))
                                        {
                                            exit(0);
                                        }
                                    }
                                    else if (strcmp("scatCounts", attr->get_name()) == 0)
                                    {
                                        LOG(WARNING) << "scatCounts: " << attr->get_value();
                                        m_freeCaseCfg.caseStripInfo[nChildCount].cbScatCount = (CT_BYTE)atoi(attr->get_value());
                                    }
                                    else if (strcmp("scatRatio", attr->get_name()) == 0)
                                    {
                                        LOG(WARNING) << "scatRatio: " << attr->get_value();
                                        std::string scatRatio = attr->get_value();
                                        std::vector<std::string> vecScatRatio;
                                        stringSplit(scatRatio, "_", vecScatRatio);
                                        if (vecScatRatio.size() != 2)
                                        {
                                            attr = pCaseStripInfoChild->next_attr();
                                            continue;
                                        }
                                        m_freeCaseCfg.caseStripInfo[nChildCount].wIconRation[0] = (CT_WORD)atoi(vecScatRatio[1].c_str());
                                    }
                                    else if (strcmp("iconWeiAreas", attr->get_name()) == 0)
                                    {
                                        LOG(WARNING) << "iconWeiAreas: " << attr->get_value();
                                        std::string iconWeiAreas = attr->get_value();
                                        std::vector<std::string> vecIconWeiAreas;
                                        stringSplit(iconWeiAreas, ";", vecIconWeiAreas);
                                        if (vecIconWeiAreas.size() != 10)
                                        {
                                            exit(0);
                                        }

                                        for (auto& it : vecIconWeiAreas)
                                        {
                                            std::string& strIconWeiAreas = it;
                                            auto pos = strIconWeiAreas.find_first_of("_");
                                            auto iconIndex = 0;
                                            auto iconRatio = 0;
                                            if (pos == std::string::npos)
                                            {
                                                exit(0);
                                            }
                                            iconIndex = atoi(strIconWeiAreas.substr(0, pos).c_str());
                                            iconRatio = atoi(strIconWeiAreas.substr(pos+1).c_str());
                                            m_freeCaseCfg.caseStripInfo[nChildCount].wIconRation[iconIndex] = iconRatio;
                                        }
                                    }
                                    else
                                    {
                                        LOG(WARNING) << "get error caseStripInfo attr. attr name: " << attr->get_name();
                                        attr = pCaseStripInfoChild->next_attr();
                                        continue;
                                    }
                                    attr = pCaseStripInfoChild->next_attr();
                                }
                                ++nChildCount;
                                pCaseStripInfoChild = pFreeReturnCaseCFGChild->next_child();
                            }
                        }
                        pFreeReturnCaseCFGChild = pFreeReturnCaseCFG->next_child();
                    }
                    break;
                }
            }
        }
    }
}