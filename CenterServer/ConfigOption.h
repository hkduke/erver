#pragma once
/*
	**充值和兑换渠道的相关配置信息
*/

#include "acl_cpp/lib_acl.hpp"
#include "GlobalStruct.h"
#include "CMD_Plaza.h"
#include <string>

class CConfigOption : public acl::singleton<CConfigOption>
{
public:
	CConfigOption();
	~CConfigOption();

public:
	CT_VOID InsertRechargeChannel(tagRechargeChannelInfo* pChannelInfo);
	CT_VOID InsertRechargeChannelAmount(tagRechargeChannelAmount* pChannelAmount);
	CT_VOID MatchRechargeChannelAmount();
	CT_VOID SendRechargeChannelInfoToClient(CT_DWORD dwUserID, CT_DWORD dwChannelID);
	
	CT_VOID InsertRechargePromoterList(std::string& promoterList);
	CT_VOID SendRechargePromoterListToClient(CT_DWORD dwUserID, CT_WORD wChannelID);
	CT_VOID	SendRechargePromoterListToClient(CT_DWORD dwUserID, CT_BYTE cbPlatformId, CT_BYTE cbProvinceId);

	CT_VOID SetExchangeChannelStatus(CT_BYTE cbExchangeChannelType, CT_BYTE cbStatus);
	CT_VOID InsertExchangeChannelAmount(tagExchangeChannelAmount* pChannelAmount);
	CT_VOID SendExchangeChannelStatusToClient(CT_DWORD dwUserID);

	CT_VOID	SetQuanMinChannelInfo(tagQuanMinChannel* pQuanMin);
	inline tagQuanMinChannel* GetQuanMinChannelInfo(CT_DWORD dwChannelID);
	
	CT_VOID SetGeneralizePromoter(tagGeneralizePromoterInfo* pPromoterInfo);
	CT_VOID SendGeneralizePromoterToClient(CT_DWORD dwUserID, CT_BYTE cbPlatformId);

	CT_VOID SetClientChannelDomain(tagClientChannelDomain* pChannelDomain);
	tagClientChannelDomain* GetClientChannelInfo(CT_DWORD dwChannelID);
	CT_VOID SendClientChannelDomain(CT_DWORD dwUserID, CT_DWORD dwChannelID);

	CT_VOID SetChannelShowExchange(tagChannelShowExchangeCond* pExchangeCond);
	CT_DWORD GetChannelShowExchangeCond(CT_DWORD wChannelID);

	CT_VOID SetVip2Config(tagVip2Config* pVip2Config);
	CT_VOID GetNextVip2LevelInfo(CT_BYTE cbCurrVip2, CT_BYTE& cbNextVip2, CT_DWORD& dwNextVip2NeedRecharge);

	CT_VOID SetBenefitReward(tagBenefitReward* pBenefitReward);
	tagBenefitReward* GetBenefitReward(CT_BYTE cbVip2);

	CT_VOID InsertPlatformRechargeCfg(tagPlatformRechargeCfg* pPlatformRechargeCfg);
	CT_VOID SendPlatformRechargeCfg(CT_DWORD dwUserID, CT_BYTE cbPlatformId);

	CT_VOID SetChatUploadUrl(tagChatUpLoadUrl* pChatUpLoadUrl);
	CT_VOID SendChatUploadUrl(CT_DWORD dwUserID);

	CT_VOID InsertAnnouncementInfo(tagAnnouncementInfo* pAnnouncementInfo);
	CT_VOID SendAnnouncementInfo(CT_DWORD dwUserID);
	CT_VOID SendAnnouncementContent(CT_DWORD dwUserID, CT_BYTE cbId);

private:
	typedef std::map<CT_BYTE, CT_BYTE> MapExchangeChannelStatus;
	
	std::map<CT_DWORD, MSG_SC_Recharge_Channel_Info> 	m_mapReChargeChannelInfo;		//渠道的充值渠道信息
	std::map<CT_BYTE, tagRechargeChannelAmount>			m_mapReChargeChannelAmount;		//充值渠道的充值金额
	//std::map<CT_WORD, std::string>						m_mapPromoterRechageList;		//代理充值的微信QQ号(暂时弃用2018-11-24)
	std::map<CT_BYTE,std::map<CT_BYTE,std::string>> 	m_mapPromoterRechargeListEx;	//代理充值的微信QQ号<--平台, --省份>
	MapExchangeChannelStatus							m_mapExchangeChannelStatus;		//兑换渠道信息
	std::map<CT_BYTE, tagExchangeChannelAmount>			m_mapExchangeChannelAmount;		//兑换渠道的兑换金额

	std::map<CT_DWORD, tagQuanMinChannel>				m_mapQuanMinChannel;			//全民代理的ID
	std::map<CT_BYTE, tagGeneralizePromoterInfo>		m_mapGeneralizePromoter;		//全民代理的代理信息
	//tagGeneralizePromoterInfo							m_GeneralizePromoter;			//全民代理的代理信息
	//CT_BYTE											m_cbGeneralizePromoterCount;	//全民代理数量

	std::map<CT_DWORD, tagClientChannelDomain>			m_mapClientChannelDomain;		//客户端渠道的主页
	std::map<CT_DWORD, tagChannelShowExchangeCond> 		m_mapChannelShowExchange; 		//渠道显示兑换
	std::map<CT_BYTE, tagVip2Config>					m_mapVip2Config;				//vip2配置
	std::map<CT_BYTE, tagBenefitReward>					m_mapBenefitReward;				//救济金奖励配置

	std::map<CT_BYTE, tagPlatformRechargeCfg>			m_mapPlatformRechargeCfg;		//平台充值信息
	std::map<CT_BYTE, std::string>						m_mapPlatformRechargeJson;		//平台充值信息的Json串

	std::vector<tagAnnouncementInfo>					m_vecAnnouncementInfo;			//公告内容

	std::string 										m_strChatUploadUrl;				//聊天上传图片地址
};