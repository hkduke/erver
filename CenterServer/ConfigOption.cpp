#include "stdafx.h"
#include "ConfigOption.h"
#include "UserMgr.h"
#include "NetModule.h"


CConfigOption::CConfigOption()
{
	/*for (int i = 0; i < 5; i++)
	{
		m_ReChargeChannelInfo.cbChannelID[i] = 0;
		m_ReChargeChannelInfo.cbState[i] = 0;
	}

	m_ReChargeChannelInfo.dwMainID = MSG_CONFIG_MSG_MAIN;
	m_ReChargeChannelInfo.dwSubID = SUB_S2C_RECHARGE_CHANNEL_INFO_MSG;*/
	m_strChatUploadUrl = "http://test.upload.lerengu.com";
}

CConfigOption::~CConfigOption()
{

}


CT_VOID CConfigOption::InsertRechargeChannel(tagRechargeChannelInfo* pChannelInfo)
{
	MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = m_mapReChargeChannelInfo[pChannelInfo->dwChannelID];
	reChargeChannelInfo.dwMainID = MSG_CONFIG_MSG_MAIN;
	reChargeChannelInfo.dwSubID = SUB_S2C_RECHARGE_CHANNEL_INFO_MSG;
	for (int i = 0; i != RECHARGE_COUNT; i++)
	{
		reChargeChannelInfo.cbChannelID[i] = pChannelInfo->cbRechargeChannelID[i];
		reChargeChannelInfo.cbState[i] = pChannelInfo->cbState[i];
	}

	if (pChannelInfo->cbReload == 1)
	{
	    MatchRechargeChannelAmount();
		SendRechargeChannelInfoToClient(0, pChannelInfo->dwChannelID);
	}
}

CT_VOID CConfigOption::InsertRechargeChannelAmount(tagRechargeChannelAmount* pChannelAmount)
{
    m_mapReChargeChannelAmount[pChannelAmount->cbChannelID] = *pChannelAmount;
}

CT_VOID CConfigOption::MatchRechargeChannelAmount()
{
    for (auto& it : m_mapReChargeChannelInfo)
    {
        MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = it.second;
        for (auto i = 0; i < RECHARGE_COUNT; ++i)
        {
            if (reChargeChannelInfo.cbChannelID[i] > RECHARGE_COUNT)
                continue;
            auto itAmount = m_mapReChargeChannelAmount.find(reChargeChannelInfo.cbChannelID[i]);
            if (itAmount != m_mapReChargeChannelAmount.end())
            {
                memcpy(reChargeChannelInfo.dwAmount[reChargeChannelInfo.cbChannelID[i] - 1], &itAmount->second.dwAmount, sizeof(reChargeChannelInfo.dwAmount[i]));
            }
        }
    }
}

CT_VOID CConfigOption::SendRechargeChannelInfoToClient(CT_DWORD dwUserID, CT_DWORD dwChannelID)
{
	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			CT_DWORD dwUserChannelID = CUserMgr::get_instance().GetUserChannelID(it.first);
			if  (dwChannelID != 0 && dwChannelID != dwUserChannelID)
			{
				continue;
			}

			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(it.first);
			if (pUserInfo == NULL)
			{
				continue;
			}

			auto itRecharge = m_mapReChargeChannelInfo.find(dwUserChannelID);
			if (itRecharge != m_mapReChargeChannelInfo.end())
			{
				MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = itRecharge->second;
				reChargeChannelInfo.dwValue2 = it.first;
				//新疆地区屏蔽代理充值以外的充值
				if (pUserInfo->cbProvinceCode == en_XINJIANG)
				{
					MSG_SC_Recharge_Channel_Info reChargeChannelInfoEx;
					memcpy(&reChargeChannelInfoEx, &reChargeChannelInfo, sizeof(reChargeChannelInfoEx));

					for(int i = 0; i < RECHARGE_COUNT; ++i)
					{
						if (reChargeChannelInfoEx.cbChannelID[i] != 5)
							reChargeChannelInfoEx.cbState[i] = 0;
					}

					CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfoEx, sizeof(reChargeChannelInfoEx));
				}
				else
					CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfo, sizeof(reChargeChannelInfo));
			}
			else
			{
				if (dwChannelID != 0)
				{
					continue;
				}
				MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = m_mapReChargeChannelInfo[0];
				reChargeChannelInfo.dwValue2 = it.first;
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfo, sizeof(reChargeChannelInfo));
			}
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
		if (pUserInfo == NULL)
		{
			return;
		}

		auto itRecharge = m_mapReChargeChannelInfo.find(dwChannelID);
		if (itRecharge != m_mapReChargeChannelInfo.end())
		{
			MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = itRecharge->second;
			reChargeChannelInfo.dwValue2 = dwUserID;

			//新疆地区屏蔽代理充值以外的充值
			if (pUserInfo->cbProvinceCode == en_XINJIANG)
			{
				MSG_SC_Recharge_Channel_Info reChargeChannelInfoEx;
				memcpy(&reChargeChannelInfoEx, &reChargeChannelInfo, sizeof(reChargeChannelInfoEx));

				for(int i = 0; i < RECHARGE_COUNT; ++i)
				{
					if (reChargeChannelInfoEx.cbChannelID[i] != 5)
						reChargeChannelInfoEx.cbState[i] = 0;
				}

				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfoEx, sizeof(reChargeChannelInfoEx));
			}
			else
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfo, sizeof(reChargeChannelInfo));
		}
		else
		{
			MSG_SC_Recharge_Channel_Info& reChargeChannelInfo = m_mapReChargeChannelInfo[0];
			reChargeChannelInfo.dwValue2 = dwUserID;

			if (pUserInfo->cbProvinceCode == en_XINJIANG)
			{
				MSG_SC_Recharge_Channel_Info reChargeChannelInfoEx;
				memcpy(&reChargeChannelInfoEx, &reChargeChannelInfo, sizeof(reChargeChannelInfoEx));

				for(int i = 0; i < RECHARGE_COUNT; ++i)
				{
					if (reChargeChannelInfoEx.cbChannelID[i] != 5)
						reChargeChannelInfoEx.cbState[i] = 0;
				}

				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfoEx, sizeof(reChargeChannelInfoEx));
			}
			else
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &reChargeChannelInfo, sizeof(reChargeChannelInfo));
		}
	}
}

CT_VOID CConfigOption::InsertRechargePromoterList(std::string& promoterList)
{
	acl::json promoterJson(promoterList.c_str());

	/*
	 * Comment in 2018-11-24
	acl::json_node* pChannelID = promoterJson.getFirstElementByTagName("code"); //code里面保存的是channelID
	if (pChannelID == NULL)
	{
		return;
	}
	acl::string strChannelID = pChannelID->get_string();
	CT_WORD wChannelID = atoi(strChannelID.c_str());

	//为什么要这样呢？因为web传过来的Json格式有点不一样，做了一个转换
	acl::string strFind;
	strFind.format("\"code\": %d", wChannelID);
	size_t it = promoterList.find_first_of(strFind.c_str());
	promoterList.erase(it, strFind.length() - 1);
	promoterList.insert(it, "\"code\": 0");

	m_mapPromoterRechageList[wChannelID] = promoterList;
	 */

	acl::json_node* pID = promoterJson.getFirstElementByTagName("code"); //code里面保存的是platformId和省份ID
	if (pID == NULL)
	{
		LOG(WARNING) << "can not find code.";
		return;
	}
	acl::string strID = pID->get_string();
	CT_DWORD wID = (CT_DWORD)atoi(strID.c_str());
	CT_BYTE cbPlatformId = (CT_BYTE)(wID / 1000);
	CT_BYTE cbProvinceId = (CT_BYTE)(wID % 1000);

	//LOG(INFO) << "InsertRechargePromoterList platformId: " << (int)cbPlatformId << ", province id: " << (int)cbProvinceId;

	std::map<CT_BYTE,std::string>& mapProvicePromoterList = m_mapPromoterRechargeListEx[cbPlatformId];
	mapProvicePromoterList[cbProvinceId] = promoterList;

    SendRechargePromoterListToClient(0, cbPlatformId, cbProvinceId);
}


/*CT_VOID CConfigOption::SendRechargePromoterListToClient(CT_DWORD dwUserID, CT_WORD wChannelID)
{
	static CT_BYTE		szBuffer[2028];
	CT_DWORD	dwSendSize = 0;

	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
	msgHead.dwSubID = SUB_S2C_RECHARGE_PROMOTER_LIST;
	//msgHead.dwValue2 = dwUserID;
	//dwSendSize += sizeof(MSG_GameMsgDownHead);

	std::string strPromoterRechargeList;
	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			//查找玩家代理ID
			CT_DWORD dwChannelID = CUserMgr::get_instance().GetUserChannelID(it.first);
			//0是默认渠道，默认渠道修改时，集体下发一次吧
			if (dwChannelID != wChannelID && wChannelID != 0)
			{
				continue;
			}

			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			msgHead.dwValue2 = it.first;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize = sizeof(MSG_GameMsgDownHead);

			//查找代理ID,如果是全民代理，也是发0的数据
			CT_BYTE cbLock = 0;
			auto itChannel = m_mapClientChannelDomain.find(dwChannelID);
			if (itChannel != m_mapClientChannelDomain.end())
			{
				cbLock = itChannel->second.cbLock;
			}

			if (cbLock == 1)
			{
				//锁定的渠道号，不用更新默认渠道
				if (wChannelID == 0)
				{
					continue;
				}

				auto itList = m_mapPromoterRechageList.find(dwChannelID);
				if (itList != m_mapPromoterRechageList.end())
				{
					strPromoterRechargeList = itList->second;
				}
				else
				{
					strPromoterRechargeList = m_mapPromoterRechageList[0];
				}
			}
			else
			{
				strPromoterRechargeList = m_mapPromoterRechageList[0];
			}


			CT_DWORD dwLen = (CT_DWORD)strPromoterRechargeList.length();
			memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
			dwSendSize += sizeof(dwLen);
			memcpy(szBuffer + dwSendSize, strPromoterRechargeList.c_str(), dwLen);
			dwSendSize += dwLen;
			szBuffer[dwSendSize] = '\0';
			dwSendSize += 1;

			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		msgHead.dwValue2 = dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize = sizeof(MSG_GameMsgDownHead);

		//查找玩家代理ID
		CT_DWORD dwChannelID = CUserMgr::get_instance().GetUserChannelID(dwUserID);

		//查找代理ID,如果是全民代理，也是发0的数据
		CT_BYTE cbLock = 0;
		auto itChannel = m_mapClientChannelDomain.find(dwChannelID);
		if (itChannel != m_mapClientChannelDomain.end())
		{
			cbLock = itChannel->second.cbLock;
		}

		if (cbLock == 1)
		{
			auto itList = m_mapPromoterRechageList.find(dwChannelID);
			if (itList != m_mapPromoterRechageList.end())
			{
				strPromoterRechargeList = itList->second;
			}
			else
			{
				strPromoterRechargeList = m_mapPromoterRechageList[0];
			}
		}
		else
		{
			strPromoterRechargeList = m_mapPromoterRechageList[0];
		}

		CT_DWORD dwLen = (CT_DWORD)strPromoterRechargeList.length();
		memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
		dwSendSize += sizeof(dwLen);
		memcpy(szBuffer + dwSendSize, strPromoterRechargeList.c_str(), dwLen);
		dwSendSize += dwLen;
		szBuffer[dwSendSize] = '\0';
		dwSendSize += 1;

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}*/

CT_VOID	CConfigOption::SendRechargePromoterListToClient(CT_DWORD dwUserID, CT_BYTE cbPlatformId, CT_BYTE cbProvinceId)
{
    auto itPlatformRchgCfg = m_mapPlatformRechargeCfg.find(cbPlatformId);
    if (itPlatformRchgCfg != m_mapPlatformRechargeCfg.end())
    {
        //如果是使用新的充值代理，则不发送数据
        if (itPlatformRchgCfg->second.cbPromoterRechargeType == 2)
        {
            return;
        }
    }

	static CT_BYTE		szBuffer[2028];
	CT_DWORD	dwSendSize = 0;

	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
	msgHead.dwSubID = SUB_S2C_RECHARGE_PROMOTER_LIST;

	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			//查找玩家省份ID
			CT_BYTE cbUserProviceId = it.second->GetProviceId();
			//如果不是同一个省份
			if (cbProvinceId != 0 && cbProvinceId != cbUserProviceId)
			{
				continue;
			}

			CT_BYTE cbUserPlatformId = it.second->GetPlatformId();
			//如果不是同一个平台
			if (cbPlatformId != 0 && cbPlatformId != cbUserPlatformId)
			{
				continue;
			}

			//平台数据
			auto itPlatformRecharge = m_mapPromoterRechargeListEx.find(cbUserPlatformId);
			if (itPlatformRecharge == m_mapPromoterRechargeListEx.end())
			{
				continue;
			}

			std::map<CT_BYTE,std::string>& mapProvincePromoter = itPlatformRecharge->second;

			//省份数据
			auto itProvince = mapProvincePromoter.find(cbUserProviceId);
			if (itProvince == mapProvincePromoter.end())
			{
				continue;
			}

			msgHead.dwValue2 = it.first;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize = sizeof(MSG_GameMsgDownHead);

			std::string& strPromoterRechargeList = itProvince->second;

			CT_DWORD dwLen = (CT_DWORD)strPromoterRechargeList.length();
			memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
			dwSendSize += sizeof(dwLen);
			memcpy(szBuffer + dwSendSize, strPromoterRechargeList.c_str(), dwLen);
			dwSendSize += dwLen;
			szBuffer[dwSendSize] = '\0';
			dwSendSize += 1;

			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		//平台数据
		auto itPlatformRecharge = m_mapPromoterRechargeListEx.find(cbPlatformId);
		if (itPlatformRecharge == m_mapPromoterRechargeListEx.end())
		{
			return;
		}

		std::map<CT_BYTE,std::string>& mapProvincePromoter = itPlatformRecharge->second;

		//省份数据
		auto itProvince = mapProvincePromoter.find(cbProvinceId);
		if (itProvince == mapProvincePromoter.end())
		{
			return;
		}

		msgHead.dwValue2 = dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize = sizeof(MSG_GameMsgDownHead);

		std::string& strPromoterRechargeList = itProvince->second;

		CT_DWORD dwLen = (CT_DWORD)strPromoterRechargeList.length();
		memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
		dwSendSize += sizeof(dwLen);
		memcpy(szBuffer + dwSendSize, strPromoterRechargeList.c_str(), dwLen);
		dwSendSize += dwLen;
		szBuffer[dwSendSize] = '\0';
		dwSendSize += 1;

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

CT_VOID CConfigOption::SetExchangeChannelStatus(CT_BYTE cbExchangeChannelType, CT_BYTE cbStatus)
{
	m_mapExchangeChannelStatus[cbExchangeChannelType] = cbStatus;

	SendExchangeChannelStatusToClient(0);
}

CT_VOID CConfigOption::InsertExchangeChannelAmount(tagExchangeChannelAmount* pChannelAmount)
{
    m_mapExchangeChannelAmount[pChannelAmount->cbChannelID] = *pChannelAmount;
}

CT_VOID CConfigOption::SendExchangeChannelStatusToClient(CT_DWORD dwUserID)
{
	MSG_SC_Exchange_Channel_Info exchangeChannelStatus;
	exchangeChannelStatus.dwMainID = MSG_CONFIG_MSG_MAIN;
	exchangeChannelStatus.dwSubID = SUB_S2C_EXCHANGE_CHANNEL_STATUS;
	exchangeChannelStatus.cbChannelStatus[0] = m_mapExchangeChannelStatus[ALIPAY_CHANNEL];
	exchangeChannelStatus.cbChannelStatus[1] = m_mapExchangeChannelStatus[UNIONPAY_CHANNEL];
    auto itAmount = m_mapExchangeChannelAmount.find(ALIPAY_CHANNEL);
    if (itAmount != m_mapExchangeChannelAmount.end())
    {
        memcpy(exchangeChannelStatus.dwAmount[0],itAmount->second.dwAmount, sizeof(exchangeChannelStatus.dwAmount[0]));
        exchangeChannelStatus.wHandlingFee[0] = itAmount->second.wHandlingFee;
        exchangeChannelStatus.cbDayCountLimite[0] = itAmount->second.cbDayCountLimite;
        exchangeChannelStatus.dwMinAmount[0] = itAmount->second.dwMinAmount;
        exchangeChannelStatus.dwMaxAmount[0] = itAmount->second.dwMaxAmount;
    }
    
    itAmount = m_mapExchangeChannelAmount.find(UNIONPAY_CHANNEL);
    if (itAmount != m_mapExchangeChannelAmount.end())
    {
        memcpy(exchangeChannelStatus.dwAmount[1],itAmount->second.dwAmount, sizeof(exchangeChannelStatus.dwAmount[1]));
        exchangeChannelStatus.wHandlingFee[1] = itAmount->second.wHandlingFee;
        exchangeChannelStatus.cbDayCountLimite[1] = itAmount->second.cbDayCountLimite;
        exchangeChannelStatus.dwMinAmount[1] = itAmount->second.dwMinAmount;
        exchangeChannelStatus.dwMaxAmount[1] = itAmount->second.dwMaxAmount;
    }
    
	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			tagGlobalUserInfo* pUserinfo = CUserMgr::get_instance().GetUserInfo(it.first);
			if (pUserinfo == NULL)
			{
				continue;
			}

			if (pUserinfo->cbProvinceCode == en_XINJIANG)
			{
				exchangeChannelStatus.cbChannelStatus[0] = 0;
			}
			else
			{
				exchangeChannelStatus.cbChannelStatus[0] = m_mapExchangeChannelStatus[ALIPAY_CHANNEL];
			}

			exchangeChannelStatus.dwValue2 = it.first;
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &exchangeChannelStatus, sizeof(exchangeChannelStatus));
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		tagGlobalUserInfo* pUserinfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
		if (pUserinfo == NULL)
		{
			return;
		}

		if (pUserinfo->cbProvinceCode == en_XINJIANG)
		{
			exchangeChannelStatus.cbChannelStatus[0] = 0;
		}
		else
		{
			exchangeChannelStatus.cbChannelStatus[0] = m_mapExchangeChannelStatus[ALIPAY_CHANNEL];
		}

		exchangeChannelStatus.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &exchangeChannelStatus, sizeof(exchangeChannelStatus));
	}
}

CT_VOID	CConfigOption::SetQuanMinChannelInfo(tagQuanMinChannel* pQuanMin)
{
	if (pQuanMin->cbStatus == 1)
	{
		m_mapQuanMinChannel[pQuanMin->dwChannelID] = *pQuanMin;
	}
	else
	{
		auto it = m_mapQuanMinChannel.find(pQuanMin->dwChannelID);
		if (it != m_mapQuanMinChannel.end())
		{
			m_mapQuanMinChannel.erase(it);
		}
	}
}

tagQuanMinChannel* CConfigOption::GetQuanMinChannelInfo(CT_DWORD dwChannelID)
{
	auto it = m_mapQuanMinChannel.find(dwChannelID);
	if (it != m_mapQuanMinChannel.end())
	{
		return &it->second;
	}

	return NULL;
}

CT_VOID CConfigOption::SetGeneralizePromoter(tagGeneralizePromoterInfo* pPromoterInfo)
{
	tagGeneralizePromoterInfo& promoterInfo = m_mapGeneralizePromoter[pPromoterInfo->cbPlatformId];
	memcpy(&promoterInfo, pPromoterInfo, sizeof(tagGeneralizePromoterInfo));

	//memset(&m_GeneralizePromoter, 0, sizeof(m_GeneralizePromoter));
	//m_cbGeneralizePromoterCount = 0;
	/*for (CT_BYTE i = 0; i < 3; ++i)
	{
		auto accountLen = strlen(pPromoterInfo->szAccount[i]);
		if (accountLen != 0)
		{
			_snprintf_info(m_GeneralizePromoter.szAccount[i], sizeof(m_GeneralizePromoter.szAccount[i]), "%s", pPromoterInfo->szAccount[i]);
			_snprintf_info(m_GeneralizePromoter.szNickName[i], sizeof(m_GeneralizePromoter.szNickName[i]), "%s", pPromoterInfo->szNickName[i]);
			m_GeneralizePromoter.cbType[i] = pPromoterInfo->cbType[i];
			++m_cbGeneralizePromoterCount;
		}
	}*/
}

CT_VOID CConfigOption::SendGeneralizePromoterToClient(CT_DWORD dwUserID, CT_BYTE cbPlatformId)
{
	auto it = m_mapGeneralizePromoter.find(cbPlatformId);
	if (it == m_mapGeneralizePromoter.end())
		return;

	if (dwUserID == 0)
	{
		MSG_SC_GeneralizePromoter_Info generalizePromoterInfo;
		generalizePromoterInfo.dwMainID = MSG_CONFIG_MSG_MAIN;
		generalizePromoterInfo.dwSubID = SUB_S2C_GENERALIZE_PROMOTER;
		generalizePromoterInfo.cbPromoterCount = it->second.cbAccountCount;

		for (CT_BYTE i = 0; i < it->second.cbAccountCount; i++)
		{
			_snprintf_info(generalizePromoterInfo.szNickName[i], sizeof(generalizePromoterInfo.szNickName[i]), "%s", it->second.szNickName[i]);
			_snprintf_info(generalizePromoterInfo.szAccount[i], sizeof(generalizePromoterInfo.szAccount[i]), "%s", it->second.szAccount[i]);
			generalizePromoterInfo.cbType[i] = it->second.cbType[i];
		}

		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& itUser : mapAllGlobalUser)
		{
			tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(itUser.first);
			if (!pUserInfo || !pUserInfo->bOnline)
			{
				continue;
			}

			if (pUserInfo->cbPlatformId != cbPlatformId)
			{
				continue;
			}

			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(itUser.first);
			if (pSocket == NULL)
			{
				continue;
			}

			CT_DWORD dwChannelID = CUserMgr::get_instance().GetUserChannelID(itUser.first);
			tagQuanMinChannel* pQuqnMinChannel = GetQuanMinChannelInfo(dwChannelID);
			if (pQuqnMinChannel != NULL)
			{
				continue;
			}

			generalizePromoterInfo.dwValue2 = itUser.first;
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &generalizePromoterInfo, sizeof(generalizePromoterInfo));
		}
	}
	else
	{
		if (it->second.cbAccountCount == 0)
			return;

		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		CT_DWORD dwChannelID = CUserMgr::get_instance().GetUserChannelID(dwUserID);
		tagQuanMinChannel* pQuqnMinChannel = GetQuanMinChannelInfo(dwChannelID);
		if (pQuqnMinChannel != NULL)
		{
			MSG_SC_GeneralizePromoter_Status quanMinStatus;
			quanMinStatus.dwMainID = MSG_CONFIG_MSG_MAIN;
			quanMinStatus.dwSubID = SUB_S2C_GENERALIZE_PROMOTER_STATUS;
			quanMinStatus.dwValue2 = dwUserID;
			quanMinStatus.cbStatus = 0;
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &quanMinStatus, sizeof(quanMinStatus));
			return;
		}
		
		MSG_SC_GeneralizePromoter_Info generalizePromoterInfo;
		generalizePromoterInfo.dwMainID = MSG_CONFIG_MSG_MAIN;
		generalizePromoterInfo.dwSubID = SUB_S2C_GENERALIZE_PROMOTER;
		generalizePromoterInfo.cbPromoterCount = it->second.cbAccountCount;

		for (CT_BYTE i = 0; i < it->second.cbAccountCount; i++)
		{
			_snprintf_info(generalizePromoterInfo.szNickName[i], sizeof(generalizePromoterInfo.szNickName[i]), "%s", it->second.szNickName[i]);
			_snprintf_info(generalizePromoterInfo.szAccount[i], sizeof(generalizePromoterInfo.szAccount[i]), "%s", it->second.szAccount[i]);
			generalizePromoterInfo.cbType[i] = it->second.cbType[i];
		}

		generalizePromoterInfo.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &generalizePromoterInfo, sizeof(generalizePromoterInfo));

		//非全民和转换用户发送公会开关
        MSG_SC_Show_GroupBtn showGroupBtn;
        showGroupBtn.dwMainID = MSG_GROUP_MAIN;
        showGroupBtn.dwSubID = SUB_SC_SHOW_GROUP_BTN;
        showGroupBtn.cbShow = 1;
        showGroupBtn.dwValue2 = dwUserID;
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &showGroupBtn, sizeof(MSG_SC_Show_GroupBtn));
    }
}

CT_VOID CConfigOption::SetClientChannelDomain(tagClientChannelDomain* pChannelDomain)
{
	m_mapClientChannelDomain[pChannelDomain->dwChannelID] = *pChannelDomain;
}

tagClientChannelDomain* CConfigOption::GetClientChannelInfo(CT_DWORD dwChannelID)
{
	auto it = m_mapClientChannelDomain.find(dwChannelID);
	if (it != m_mapClientChannelDomain.end())
	{
		return &it->second;
	}

	return NULL;
}

CT_VOID CConfigOption::SendClientChannelDomain(CT_DWORD dwUserID, CT_DWORD dwChannelID)
{
	//CT_BYTE		szBuffer[1024];
	//CT_DWORD	dwSendSize = 0;

	MSG_SC_ClientChannel_Domain channelDomain;
	channelDomain.dwMainID = MSG_CONFIG_MSG_MAIN;
	channelDomain.dwSubID = SUB_S2C_CLIENT_CHANNEL_DOMAIN;

	std::string strPromoterRechargeList;
	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			//查找玩家代理ID
			CT_DWORD dwUserChannelID = CUserMgr::get_instance().GetUserClientChannelID(it.first);

			if (dwUserChannelID != dwChannelID)
			{
				continue;
			}

			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			channelDomain.dwValue2 = it.first;

			//查找代理ID
			auto itChannel = m_mapClientChannelDomain.find(dwUserChannelID);
			if (itChannel != m_mapClientChannelDomain.end())
			{
				_snprintf_info(channelDomain.szChannelDomain, sizeof(channelDomain.szChannelDomain), "%s", itChannel->second.szDomain);
				_snprintf_info(channelDomain.szChannelDomain2, sizeof(channelDomain.szChannelDomain2), "%s", itChannel->second.szDomain2);
			}
			else
			{
				_snprintf_info(channelDomain.szChannelDomain, sizeof(channelDomain.szChannelDomain), "%s", "www.xingkong1.com");
				_snprintf_info(channelDomain.szChannelDomain2, sizeof(channelDomain.szChannelDomain2), "%s", "729.me/in.html?enterprise=1");
			}

			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &channelDomain, sizeof(channelDomain));
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		channelDomain.dwValue2 = dwUserID;
		//查找玩家代理ID
		CT_DWORD dwUserChannelID = CUserMgr::get_instance().GetUserClientChannelID(dwUserID);

		//查找代理ID
		auto itChannel = m_mapClientChannelDomain.find(dwUserChannelID);
		if (itChannel != m_mapClientChannelDomain.end())
		{
			_snprintf_info(channelDomain.szChannelDomain, sizeof(channelDomain.szChannelDomain), "%s", itChannel->second.szDomain);
			_snprintf_info(channelDomain.szChannelDomain2, sizeof(channelDomain.szChannelDomain2), "%s", itChannel->second.szDomain2);
		}
		else
		{
			_snprintf_info(channelDomain.szChannelDomain, sizeof(channelDomain.szChannelDomain), "%s", "www.xingkong1.com");
			_snprintf_info(channelDomain.szChannelDomain2, sizeof(channelDomain.szChannelDomain2), "%s", "729.me/in.html?enterprise=1");
		}

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &channelDomain, sizeof(channelDomain));
	}
}

CT_VOID CConfigOption::SetChannelShowExchange(tagChannelShowExchangeCond* pExchangeCond)
{
	if (pExchangeCond->dwChannelID == 0)
	{
		return;
	}
	
	m_mapChannelShowExchange[pExchangeCond->dwChannelID] = *pExchangeCond;
}

CT_DWORD CConfigOption::GetChannelShowExchangeCond(CT_DWORD wChannelID)
{
	auto it = m_mapChannelShowExchange.find(wChannelID);
	if (it != m_mapChannelShowExchange.end())
	{
		return it->second.dwTotalRecharge;
	}

	return 0;
}

CT_VOID CConfigOption::SetVip2Config(tagVip2Config* pVip2Config)
{
	m_mapVip2Config[pVip2Config->cbVip2] = *pVip2Config;
}

CT_VOID CConfigOption::GetNextVip2LevelInfo(CT_BYTE cbCurrVip2, CT_BYTE& cbNextVip2, CT_DWORD& dwNextVip2NeedRecharge)
{
	auto it = m_mapVip2Config.find(cbCurrVip2 + 1);
	if (it != m_mapVip2Config.end())
	{
		cbNextVip2 = it->first;
		dwNextVip2NeedRecharge = it->second.dwRecharge;
	}
	else
	{
		cbNextVip2 = cbCurrVip2;
	}
}

CT_VOID CConfigOption::SetBenefitReward(tagBenefitReward* pBenefitReward)
{
	m_mapBenefitReward[pBenefitReward->cbVip2] = *pBenefitReward;
}

tagBenefitReward* CConfigOption::GetBenefitReward(CT_BYTE cbVip2)
{
    auto it = m_mapBenefitReward.find(cbVip2);
    if (it != m_mapBenefitReward.end())
    {
        return &it->second;
    }

    return NULL;
}

CT_VOID CConfigOption::InsertPlatformRechargeCfg(tagPlatformRechargeCfg* pPlatformRechargeCfg)
{
	auto it = m_mapPlatformRechargeCfg.find(pPlatformRechargeCfg->cbPlatformId);
	CT_BOOL bSendRechargePromoterList = false;
	if (it != m_mapPlatformRechargeCfg.end())
	{
		//判断老的配置和新的配置有什么不同
		//当由新充值转到老充值的时候，要广播一下
		if (it->second.cbPromoterRechargeType == 2 && pPlatformRechargeCfg->cbPromoterRechargeType == 1)
        {
            bSendRechargePromoterList = true;
        }
	}

	//无论新旧配置都直接赋值
	m_mapPlatformRechargeCfg[pPlatformRechargeCfg->cbPlatformId] = *pPlatformRechargeCfg;

	//生成json串
	acl::json jsonObject;

	acl::json_node& root = jsonObject.get_root();
	acl::json_node *node0, *node1, *node2, *node3, *node4, *node5;

	node0 = &root;

	node1 = &jsonObject.create_node("RchgDomain", pPlatformRechargeCfg->szRechargeDomain);
	node0->add_child(node1);

#ifdef _OS_WIN32_CODE
	node2 = &jsonObject.create_node("PRchgType", (__int64)pPlatformRechargeCfg->cbPromoterRechargeType);
#else
	node2 = &jsonObject.create_node("PRchgType", (long long int)pPlatformRechargeCfg->cbPromoterRechargeType);
#endif
	node0->add_child(node2);

	node3 = &jsonObject.create_node("PRchgUrl", pPlatformRechargeCfg->szPromoterRechargeUrl);
	node0->add_child(node3);

	//node3 = &jsonObject.create_node("PRchgUpload", pPlatformRechargeCfg->szPromoterRechargeUpload);
	//node0->add_child(node3);

	node4 = &jsonObject.create_node("PRchgMerchant", pPlatformRechargeCfg->szPromoterRechargeMerchant);
	node0->add_child(node4);

	node5 = &jsonObject.create_node("PRchgToken", pPlatformRechargeCfg->szPromoterRechargeToken);
	node0->add_child(node5);

	acl::string strJson;
	jsonObject.build_json(strJson);

	m_mapPlatformRechargeJson[pPlatformRechargeCfg->cbPlatformId] = std::string(strJson.c_str());

    //由新代理转换到老代理充值的时候，要广播一次代理充值微信号
    if (bSendRechargePromoterList)
        SendRechargePromoterListToClient(0, 0, 0);

    //需要广播一次这个信息
    SendPlatformRechargeCfg(0, pPlatformRechargeCfg->cbPlatformId);
}

CT_VOID CConfigOption::SendPlatformRechargeCfg(CT_DWORD dwUserID, CT_BYTE cbPlatformId)
{
	static CT_BYTE	szBuffer[2048];
	CT_DWORD	dwSendSize = 0;

	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
	msgHead.dwSubID = SUB_S2C_PLATFORM_RECHARGE_CFG;

	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}

			CT_BYTE cbUserPlatformId = it.second->GetPlatformId();

			//如果不是同一个平台
			if (cbPlatformId != cbUserPlatformId)
			{
				continue;
			}

			//平台数据
			auto itPlatformRechargeCfg = m_mapPlatformRechargeJson.find(cbUserPlatformId);
			if (itPlatformRechargeCfg == m_mapPlatformRechargeJson.end())
			{
				continue;
			}

			msgHead.dwValue2 = it.first;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize = sizeof(MSG_GameMsgDownHead);

			std::string& strPlatformRechargeCfg = itPlatformRechargeCfg->second;

			CT_DWORD dwLen = (CT_DWORD)strPlatformRechargeCfg.length();
			memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
			dwSendSize += sizeof(dwLen);
			memcpy(szBuffer + dwSendSize, strPlatformRechargeCfg.c_str(), dwLen);
			dwSendSize += dwLen;

			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}

		//平台数据
		auto itPlatformRechargeCfg = m_mapPlatformRechargeJson.find(cbPlatformId);
		if (itPlatformRechargeCfg == m_mapPlatformRechargeJson.end())
		{
			return;
		}

		msgHead.dwValue2 = dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize = sizeof(MSG_GameMsgDownHead);

		std::string& strPlatformRechargeCfg = itPlatformRechargeCfg->second;

		CT_DWORD dwLen = (CT_DWORD)strPlatformRechargeCfg.length();
		memcpy(szBuffer + dwSendSize, &dwLen, sizeof(dwLen));
		dwSendSize += sizeof(dwLen);
		memcpy(szBuffer + dwSendSize, strPlatformRechargeCfg.c_str(), dwLen);
		dwSendSize += dwLen;

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

CT_VOID CConfigOption::SetChatUploadUrl(tagChatUpLoadUrl* pChatUpLoadUrl)
{
    m_strChatUploadUrl = pChatUpLoadUrl->szUploadUrl;
    LOG(WARNING) << "set chat upload url: " << m_strChatUploadUrl.c_str();
}

CT_VOID CConfigOption::SendChatUploadUrl(CT_DWORD dwUserID)
{
    CT_BYTE	    szBuffer[1048];
    CT_DWORD	dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
    msgHead.dwSubID = SUB_S2C_QUERY_CHATUPLOAD_URL;

    acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
    if (pSocket == NULL)
    {
        return;
    }

    msgHead.dwValue2 = dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize = sizeof(MSG_GameMsgDownHead);

    CT_WORD wLen = (CT_WORD)m_strChatUploadUrl.length();
    memcpy(szBuffer + dwSendSize, &wLen, sizeof(wLen));
    dwSendSize += sizeof(wLen);
    memcpy(szBuffer + dwSendSize, m_strChatUploadUrl.c_str(), wLen);
    dwSendSize += wLen;

    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}


CT_VOID CConfigOption::InsertAnnouncementInfo(tagAnnouncementInfo* pAnnouncementInfo)
{
	for (auto it = m_vecAnnouncementInfo.begin(); it != m_vecAnnouncementInfo.end(); ++it)
	{
		if (it->cbAnnouncementID == pAnnouncementInfo->cbAnnouncementID)
		{
			if (pAnnouncementInfo->cbStatus != 1)
			{
				m_vecAnnouncementInfo.erase(it);
				SendAnnouncementInfo(0);
				return;
			}

		    it->cbSortID = pAnnouncementInfo->cbSortID;
		    _snprintf_info(it->szName, sizeof(it->szName), "%s", pAnnouncementInfo->szName);
            _snprintf_info(it->szContent, sizeof(it->szContent), "%s", pAnnouncementInfo->szContent);
            std::sort(m_vecAnnouncementInfo.begin(), m_vecAnnouncementInfo.end(), [](tagAnnouncementInfo& a, tagAnnouncementInfo& b)
            {
                return a.cbSortID < b.cbSortID;
            });
            SendAnnouncementInfo(0);
            SendAnnouncementContent(0, it->cbAnnouncementID);
			return;
		}
	}

	//如果是增加无效的公告,则不插入
	if (pAnnouncementInfo->cbStatus != 1)
		return;

	m_vecAnnouncementInfo.push_back(*pAnnouncementInfo);
	std::sort(m_vecAnnouncementInfo.begin(), m_vecAnnouncementInfo.end(), [](tagAnnouncementInfo& a, tagAnnouncementInfo& b)
	{
		return a.cbSortID < b.cbSortID;
	});

	//简单粗暴的广播一次(原因后台新增的时候，需要实时通知客户端)
	SendAnnouncementInfo(0);
}

CT_VOID CConfigOption::SendAnnouncementInfo(CT_DWORD dwUserID)
{
    if (dwUserID != 0)
    {
        acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
        if (pSocket == NULL)
        {
            return;
        }

        CT_BYTE	    szBuffer[256];
        CT_DWORD	dwSendSize = 0;

        MSG_GameMsgDownHead msgHead;
        msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
        msgHead.dwSubID = SUB_S2C_ANNOUNCEMENT_INFO;

        msgHead.dwValue2 = dwUserID;
        memcpy(szBuffer, &msgHead, sizeof(msgHead));
        dwSendSize += sizeof(MSG_GameMsgDownHead);

        MSG_SC_Announcement_Info announcementInfo;
        for (auto& it : m_vecAnnouncementInfo)
        {
            announcementInfo.cbAnnouncementID = it.cbAnnouncementID;
            _snprintf_info(announcementInfo.szAnnounceName, sizeof(announcementInfo.szAnnounceName), "%s", it.szName);
            memcpy(szBuffer + dwSendSize, &announcementInfo, sizeof(MSG_SC_Announcement_Info));
            dwSendSize += sizeof(MSG_SC_Announcement_Info);
        }

        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
    }
    else
    {
        MapGlobalUser &mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto &it : mapAllGlobalUser)
        {
            acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
            if (pSocket == NULL)
            {
                continue;
            }

            CT_BYTE szBuffer[256];
            CT_DWORD dwSendSize = 0;

            MSG_GameMsgDownHead msgHead;
            msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
            msgHead.dwSubID = SUB_S2C_ANNOUNCEMENT_INFO;

            msgHead.dwValue2 = it.first;
            memcpy(szBuffer, &msgHead, sizeof(msgHead));
            dwSendSize += sizeof(MSG_GameMsgDownHead);

            MSG_SC_Announcement_Info announcementInfo;
            for (auto &it : m_vecAnnouncementInfo)
            {
                announcementInfo.cbAnnouncementID = it.cbAnnouncementID;
                _snprintf_info(announcementInfo.szAnnounceName,
                               sizeof(announcementInfo.szAnnounceName),
                               "%s",
                               it.szName);
                memcpy(szBuffer + dwSendSize, &announcementInfo, sizeof(MSG_SC_Announcement_Info));
                dwSendSize += sizeof(MSG_SC_Announcement_Info);
            }

            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
        }
    }
}

CT_VOID CConfigOption::SendAnnouncementContent(CT_DWORD dwUserID, CT_BYTE cbId)
{
    if (dwUserID != 0)
    {
        acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
        if (pSocket == NULL)
        {
            return;
        }

        for (auto& it : m_vecAnnouncementInfo)
        {
            if (it.cbAnnouncementID == cbId)
            {
				static CT_BYTE	szBuffer[SYS_NET_SENDBUF_CLIENT];
				CT_DWORD	    dwSendSize = 0;

				MSG_GameMsgDownHead msgHead;
				msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
				msgHead.dwSubID = SUB_S2C_ANNOUNCEMENT_CONTENT;
				msgHead.dwValue2 = dwUserID;
				memcpy(szBuffer, &msgHead, sizeof(msgHead));
				dwSendSize += sizeof(MSG_GameMsgDownHead);

				memcpy(szBuffer + dwSendSize, &cbId, sizeof(CT_BYTE));
				dwSendSize += sizeof(CT_BYTE);

                CT_DWORD dwContentLen = strlen(it.szContent);
                memcpy(szBuffer + dwSendSize, &dwContentLen, sizeof(CT_DWORD));
                dwSendSize += sizeof(CT_DWORD);
                memcpy(szBuffer + dwSendSize, &it.szContent, dwContentLen);
                dwSendSize += dwContentLen;
                CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
                break;
            }
        }
    }
    else
    {
        MapGlobalUser &mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
        for (auto &it : mapAllGlobalUser)
        {
            acl::aio_socket_stream *pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
            if (pSocket == NULL)
            {
                continue;
            }

            for (auto& it : m_vecAnnouncementInfo)
            {
                if (it.cbAnnouncementID == cbId)
                {
					CT_BYTE	szBuffer[SYS_NET_SENDBUF_CLIENT];
					CT_DWORD	    dwSendSize = 0;

					MSG_GameMsgDownHead msgHead;
					msgHead.dwMainID = MSG_CONFIG_MSG_MAIN;
					msgHead.dwSubID = SUB_S2C_ANNOUNCEMENT_CONTENT;
					msgHead.dwValue2 = dwUserID;
					memcpy(szBuffer, &msgHead, sizeof(msgHead));
					dwSendSize += sizeof(MSG_GameMsgDownHead);

					memcpy(szBuffer + dwSendSize, &cbId, sizeof(CT_BYTE));
					dwSendSize += sizeof(CT_BYTE);

                    CT_DWORD dwContentLen = strlen(it.szContent);
                    memcpy(szBuffer + dwSendSize, &dwContentLen, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                    memcpy(szBuffer + dwSendSize, &it.szContent, dwContentLen);
                    dwSendSize += dwContentLen;
                    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
                    break;
                }
            }
        }
    }
}
