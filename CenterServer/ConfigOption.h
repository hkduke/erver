#pragma once
/*
	**��ֵ�Ͷһ����������������Ϣ
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
	
	std::map<CT_DWORD, MSG_SC_Recharge_Channel_Info> 	m_mapReChargeChannelInfo;		//�����ĳ�ֵ������Ϣ
	std::map<CT_BYTE, tagRechargeChannelAmount>			m_mapReChargeChannelAmount;		//��ֵ�����ĳ�ֵ���
	//std::map<CT_WORD, std::string>						m_mapPromoterRechageList;		//�����ֵ��΢��QQ��(��ʱ����2018-11-24)
	std::map<CT_BYTE,std::map<CT_BYTE,std::string>> 	m_mapPromoterRechargeListEx;	//�����ֵ��΢��QQ��<--ƽ̨, --ʡ��>
	MapExchangeChannelStatus							m_mapExchangeChannelStatus;		//�һ�������Ϣ
	std::map<CT_BYTE, tagExchangeChannelAmount>			m_mapExchangeChannelAmount;		//�һ������Ķһ����

	std::map<CT_DWORD, tagQuanMinChannel>				m_mapQuanMinChannel;			//ȫ������ID
	std::map<CT_BYTE, tagGeneralizePromoterInfo>		m_mapGeneralizePromoter;		//ȫ�����Ĵ�����Ϣ
	//tagGeneralizePromoterInfo							m_GeneralizePromoter;			//ȫ�����Ĵ�����Ϣ
	//CT_BYTE											m_cbGeneralizePromoterCount;	//ȫ���������

	std::map<CT_DWORD, tagClientChannelDomain>			m_mapClientChannelDomain;		//�ͻ�����������ҳ
	std::map<CT_DWORD, tagChannelShowExchangeCond> 		m_mapChannelShowExchange; 		//������ʾ�һ�
	std::map<CT_BYTE, tagVip2Config>					m_mapVip2Config;				//vip2����
	std::map<CT_BYTE, tagBenefitReward>					m_mapBenefitReward;				//�ȼý�������

	std::map<CT_BYTE, tagPlatformRechargeCfg>			m_mapPlatformRechargeCfg;		//ƽ̨��ֵ��Ϣ
	std::map<CT_BYTE, std::string>						m_mapPlatformRechargeJson;		//ƽ̨��ֵ��Ϣ��Json��

	std::vector<tagAnnouncementInfo>					m_vecAnnouncementInfo;			//��������

	std::string 										m_strChatUploadUrl;				//�����ϴ�ͼƬ��ַ
};