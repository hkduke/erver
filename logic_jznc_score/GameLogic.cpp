#include "GameLogic.h"
#include <time.h>
#include <math.h>
#include "glog_wrapper.h"
#include <string.h>
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>
#include <cstdlib>

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////


//构造函数
CGameLogic::CGameLogic():m_gen(m_rd()), m_iProba(0,99)
{
    srand(time(NULL));
    std::srand(time(NULL));
    m_mCtrlEffect.clear();
    m_mEffectDice.clear();
}

//析构函数
CGameLogic::~CGameLogic()
{
    m_mCtrlEffect.clear();
    m_mEffectDice.clear();
}
void CGameLogic::SetCtrlEffect(std::map<int , std::vector<CT_WORD >> & ctrlEffect)
{
    m_mCtrlEffect.clear();
    std::copy(ctrlEffect.begin(),ctrlEffect.end(),std::inserter(m_mCtrlEffect,m_mCtrlEffect.begin()));
    m_mEffectDice.clear();
    for (auto i = ctrlEffect.begin(); i != ctrlEffect.end(); ++i)
    {
        FillDice(i->first);
    }
    return;
}
CT_VOID CGameLogic::FillDice(int key)
{
    auto pCtrlEffect = m_mCtrlEffect.find(key);
    if(pCtrlEffect == m_mCtrlEffect.end())
    {
        LOG(ERROR) << "m_mCtrlEffect key:" << key;
        return;
    }

    std::vector<CT_WORD > vecDice;
    CT_WORD wTotal = 0;
    for (CT_WORD j = 0; j < pCtrlEffect->second.size() && (pCtrlEffect->second.size() == 6); ++j)
    {
        wTotal += pCtrlEffect->second[j];
        for (CT_WORD k = 0; k < pCtrlEffect->second[j]; ++k)
        {
            vecDice.push_back(j+1);
        }
    }
    if(vecDice.size() != wTotal || wTotal < 6)
    {
        vecDice.clear();
        for (CT_WORD j = 0; j < 6; ++j)
        {
            vecDice.push_back(j+1);
        }
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(vecDice.begin(), vecDice.end(), std::default_random_engine(seed));
    m_mEffectDice[key].assign(vecDice.begin(), vecDice.end());
}
CT_WORD CGameLogic::RandDice(int key)
{
    auto pdice = m_mEffectDice.find(key);
    if(pdice == m_mEffectDice.end())
    {
        LOG(ERROR) << "RandDice key:" << key;
        std::uniform_int_distribution<unsigned int> disValue(1,6);
        return (CT_WORD)disValue(m_gen);
    }
    if(pdice->second.size() <= 0)
    {
        FillDice(key);
    }
    if(pdice->second.size() <= 0)
    {
        LOG(ERROR) << "RandDice pdice->second.size() == 0 key:" << key;
        std::uniform_int_distribution<unsigned int> disValue(1,6);
        return (CT_WORD)disValue(m_gen);
    }
    //LOG(INFO) << "RandDice key:"<<key;
    std::uniform_int_distribution<unsigned int> disValue(0,pdice->second.size()-1);
    CT_WORD wIndex = disValue(m_gen);
    CT_WORD wDice = pdice->second.at(wIndex);
    pdice->second.erase(pdice->second.begin()+wIndex);
    return wDice;
}