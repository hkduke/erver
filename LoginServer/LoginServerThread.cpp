#include "LoginServerThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "ServerMgr.h"
#include "CMD_Plaza.h"
#include "CMD_Game.h"
#include "GlobalEnum.h"
#include "Utility.h"
#include <random>
#include "timeFunction.h"

//中心服务器连接
extern CNetConnector *pNetCenter;

//定时器
#define		IDI_TIMER_TEST						(100)
#define		TIMER_TEST							(2000)

#define		IDI_CONNECT_CENTER					(101)
#define		TIME_CONNECT_CENTER					(10000)

#define		IDI_TIMER_INIT_SERVER				(102)
#define		TIMER_INIT_SERVER					(1000)

#define		IDI_TIMER_ACROSS_DAY				(103)
#define		TIME_ACROSS_DAY						(60*1000)

#define		MAX_RAND_USERID						100000


CLoginServerThread::CLoginServerThread(void)
	: m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	, m_redisClient2(CServerCfg::m_RedisAddress2, CServerCfg::m_RedisConTimeOut2, CServerCfg::m_RedisRwTimeOut2)
	, m_pIpFinder("qqwry.dat")
	, m_iRegisterPresentGem(0)
	, m_iRegisterPresentScore(0)
	, m_iBindMobileScore(0)
	//, m_cbSignInStatus(0)
	//, m_cbBenefitStatus(0)
	, m_cbOpenApplePay(0)
	, m_wIpRegisterLimite(0)
	, m_bInitServer(false)
	, m_iTodayTime((CT_INT64)time(NULL))
{
	acl::string charset;
#ifdef _OS_WIN32_CODE
	acl::db_handle::set_loadpath("libmysql.dll");
	charset.format("%s", "gbk");
#elif defined(_OS_LINUX_CODE)
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.so");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#else
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.dylib");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#endif

	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbUser, CServerCfg::m_AccountdbPwd, 2, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbUser, CServerCfg::m_RecorddbPwd, 2, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbUser, CServerCfg::m_PlatformdbPwd, 2, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_GamedbIP, CServerCfg::m_GamedbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_GamedbName, CServerCfg::m_GamedbUser, CServerCfg::m_GamedbPwd, 2, 0, true, 60, 60, charset.c_str());

	m_accountDBKey.format("%s@%s:%d", CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_recordDBKey.format("%s@%s:%d", CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	m_gameDBKey.format("%s@%s:%d", CServerCfg::m_GamedbName, CServerCfg::m_GamedbIP, CServerCfg::m_GamedbPort);

	dbaddr.format("%s:%d", CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	acl::mysql_conf dbconf(dbaddr, CServerCfg::m_AccountdbName);
	dbconf.set_dbuser(CServerCfg::m_AccountdbUser).set_dbpass(CServerCfg::m_AccountdbPwd).\
		set_dblimit(2).set_auto_commit(false).set_rw_timeout(60).set_charset(charset.c_str());

	std::unique_ptr<acl::mysql_pool> dbMysqlPool (new acl::mysql_pool(dbconf));
	m_accountDBPool = std::move(dbMysqlPool);

	m_mapProvince2Id["北京"]  = en_BEIJING;
    m_mapProvince2Id["天津"]  = en_TIANJIN;
    m_mapProvince2Id["河北"]  = en_HEBEI;
    m_mapProvince2Id["山西"]  = en_SHANXI;
    m_mapProvince2Id["内蒙古"] = en_NEIMENGGU;
    m_mapProvince2Id["辽宁"]  = en_LIAONING;
    m_mapProvince2Id["吉林"]  = en_JILIN;
    m_mapProvince2Id["黑龙江"] = en_HEILONGJIANG;
    m_mapProvince2Id["上海"]  = en_SHANGHAI;
    m_mapProvince2Id["江苏"]  = en_JIANGSU;
    m_mapProvince2Id["浙江"]  = en_ZHEJIANG;
    m_mapProvince2Id["安徽"]  = en_ANHUI;
    m_mapProvince2Id["福建"]  = en_FUJIAN;
    m_mapProvince2Id["江西"]  = en_JIANGXI;
    m_mapProvince2Id["山东"]  = en_SHANDONG;
    m_mapProvince2Id["河南"]  = en_HENAN;
    m_mapProvince2Id["湖北"]  = en_HUBEI;
    m_mapProvince2Id["湖南"]  = en_HUNAN;
    m_mapProvince2Id["广东"]  = en_GUANGDONG;
    m_mapProvince2Id["广西"]  = en_GUANGXI;
    m_mapProvince2Id["海南"]  = en_HAINAN;
    m_mapProvince2Id["重庆"]  = en_CHONGQING;
    m_mapProvince2Id["四川"]  = en_SICHUAN;
    m_mapProvince2Id["贵州"]  = en_GUIZHOU;
    m_mapProvince2Id["云南"]  = en_YUNNAN;
    m_mapProvince2Id["西藏"]  = en_XIZANG;
    m_mapProvince2Id["陕西"]  = en_SHANXI2;
    m_mapProvince2Id["甘肃"]  = en_GANSU;
    m_mapProvince2Id["青海"]  = en_QINGHAI;
    m_mapProvince2Id["宁夏"]  = en_NINGXIA;
    m_mapProvince2Id["新疆"]  = en_XINJIANG;
    //m_mapProvince2Id["台湾"]  = en_TAIWAN;
    //m_mapProvince2Id["香港"]  = en_XIANGGANG;
   // m_mapProvince2Id["澳门"]  = en_AOMEN;
    //m_mapProvince2Id["国外"]  = en_GUOWAI;
    //m_mapProvince2Id["其它"]  = en_OTHER;
}

CLoginServerThread::~CLoginServerThread(void)
{

}

CT_VOID CLoginServerThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	stConnectParam& connParam = pConnector->GetConnectParam();
	if (connParam.conType == CONNECT_CENTER)
	{
		if (!m_bInitServer)
		{
			InitServer();
			m_bInitServer = CT_TRUE;
			//CNetModule::getSingleton().SetTimer(IDI_TIMER_TEST, 1000, this, false);
			//暂时让10号登录服做跨天操作
			if (CServerCfg::m_nServerID == 10)
			{
				CNetModule::getSingleton().SetTimer(IDI_TIMER_ACROSS_DAY, TIME_ACROSS_DAY, this, true);
			}

			std::unique_ptr<CLoginRecordThread> loginRecordThread(new CLoginRecordThread(false));
			m_LoginRecordThreadPtr = std::move(loginRecordThread);

			m_LoginRecordThreadPtr->set_detachable(true);
			if (m_LoginRecordThreadPtr->start() == false)
			{
				LOG(WARNING) << "start login record thread failed!";
			}

            /*std::unique_ptr<CVerificationCodeThread> verificationCodeThread(new CVerificationCodeThread(false));
            m_VerificationCodeThreadPtr = std::move(verificationCodeThread);

            m_VerificationCodeThreadPtr->set_detachable(true);
            if (m_VerificationCodeThreadPtr->start() == false)
            {
                LOG(WARNING) << "start verification code thread failed!";
            }*/

			/*std::unique_ptr<CExchangePostThread> exchangePostThread (new CExchangePostThread(false));
			m_ExchangePostThreadPtr = std::move(exchangePostThread);

			m_ExchangePostThreadPtr->set_detachable(true);
			if (m_ExchangePostThreadPtr->start() == false)
			{
				LOG(WARNING) << "start exchange post thread failed!";
			}*/
		}
		GoCenterServerRegister();
	}
}

CT_VOID CLoginServerThread::OnTCPSocketShut(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_CENTER)
	{
		LOG(WARNING) << "center server is disconnect, after " << TIME_CONNECT_CENTER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_CENTER, TIME_CONNECT_CENTER, this, false);
	}
}

CT_VOID CLoginServerThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{
	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "bind one connect: " <<  strIp;// pSocket->get_peer(true);
	}
}

CT_VOID CLoginServerThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	if (pSocket)
	{
		CServerMgr::get_instance().DeleteProxyServer(pSocket);
	}
}

//网络bit数据到来
CT_VOID CLoginServerThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case MSG_PLS_MAIN:
	{
		OnProxyServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;

	case MSG_LOGIN_MAIN:
	{
		OnUserLoginMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;

	case MSG_LCS_MAIN:
	{
		OnCenterServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;

	default:
		break;
	}
}


//网络定时器
CT_VOID CLoginServerThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_TIMER_TEST:
	{
		LOG(INFO) << "timer test...";

		/*MSG_CS_SaveScore_To_Bank saveBank;
		saveBank.dwUserID = 80154171;
		saveBank.dSaveScore = 9223372036854775808;
		 -92233720368547760
		OnUserSaveBankScore(&saveBank, sizeof(saveBank), NULL);*/
		

		/*acl::db_pool* pool = GetAccountdbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get account db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get account db handle fail.";
			return;
		}
		
		MSG_CS_Bind_Mobile bindMobile;
		bindMobile.dwUserID = 10185;
		_snprintf_info(bindMobile.szMobileNum, sizeof(bindMobile.szMobileNum), "%d8", (CT_DWORD)time(NULL));
		_snprintf_info(bindMobile.szPassword, sizeof(bindMobile.szPassword), "%s", "9cbf8a4dcb8e30682b927f352d6559a0");
		 
		std::string oldAccount;
		acl::string newNickName;
		newNickName.format("%u", bindMobile.dwUserID);
		std::string newNickNameUtf8(Utility::Ansi2Utf8(newNickName.c_str()));

		CT_BYTE bResult = GoDBBindMobile(db, &bindMobile, oldAccount, newNickName);
		if (bResult != 0)
		{
			LOG(WARNING) << "bind mobile fail.";
			pool->put(db);
			return;
		}
		pool->put(db);*/
		//InsertUserID();
	}
	break;
	case IDI_CONNECT_CENTER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetCenter);
	}
	break;
	case IDI_TIMER_INIT_SERVER:
	{
		acl::db_pool* pool = GetPlatformdbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get platform db pool fail.";
			return;

		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get platform db handle fail.";
			return;
		}
		LoadGameKind(db);
		LoadGameRoomKind(db);
		LoadSystemStatus(db);
		if (m_cbScoreMode)
		{
			//LoadSignInConfig(db);
			LoadVipConfig(db);
			LoadVip2Config(db);
			LoadChannelShowExchange(db);
			LoadExchangeInfo(db);
			LoadChannelPresentScore(db);
            LoadChannelSendRegisterMail(db);
			LoadBenefitConfig(db);
			//LoadExchangeScoreConfig(db);
		}
	
		LoadGameVersion(db);
		LoadRegisterReward(db);

		pool->put(db);
		LOG(WARNING) << "loginserver start succ.";

		/*UserAccountsBaseData accountData;
		MSG_CS_LoginDataEx LoginData;  
		LoginData.cbPlatformID = 1;
		LoginData.wGameID = 1;
		LoginData.cbGender = 1;
		_snprintf_info(LoginData.chPlatformAccount, sizeof(LoginData.chPlatformAccount),	"%s", "oD7rJv3AHOMX4P2P_t_pxthOcg8M");
		_snprintf_info(LoginData.chPlatformPWD,		sizeof(LoginData.chPlatformPWD), "		%s",  "1");
		_snprintf_info(LoginData.chNickName,		sizeof(LoginData.chNickName),			"%s", "zixuekuang");
		_snprintf_info(LoginData.chHeadUrl,			sizeof(LoginData.chHeadUrl),			"%s", "http://wx.qlogo.cn/mmopen/ajNVdqHZLLAlOmsIaEv3eZVd1lRbJVALW5nXMmPQaVg5QpFiaFLicASHU4QnNNA38A9p0sgAKFzqsKMaK0dCqD5A/0");
		_snprintf_info(LoginData.chMachineSerial,	sizeof(LoginData.chMachineSerial),		"%s", "8A4239B9-DB2C-4A71-A56C-9BE9DA67EB9F");
		_snprintf_info(LoginData.chMachineType,		sizeof(LoginData.chMachineType),		"%s", "iPhone");
		_snprintf_info(LoginData.chClientIP,		sizeof(LoginData.chClientIP),			"%s", "192.168.1.100");
	
		GoDBAccountLoginOrRegister(&LoginData, accountData);*/
	}
	break;
	case IDI_TIMER_ACROSS_DAY:
	{
		AcrossTheDayEvent();
	}
	break;
	default:
		break;
	}
}


//打开
CT_VOID CLoginServerThread::Open()
{
}

CT_VOID CLoginServerThread::OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	    case SUB_CS2L_UPDATE_SHOWEXCHANGE:
	    {
	    	OnUpdateShowExchangeCond(pData, wDataSize);
            break;
	    }
	    case SUB_CS2L_UPDATE_CHANNLE_PRESENT_SCORE:
	    {
	    	OnUpdateChannelPresentScore(pData, wDataSize);
            break;
	    }
	    case SUB_CS2L_UPDATE_EXCHANGE_INFO:
	    {
	    	OnUpdateExchangeInfo(pData, wDataSize);
            break;
	    }
        case SUB_CS2L_UPDATE_BENEFIT_CONFIG:
        {
	    	OnUpdateBenefitConfig(pData, wDataSize);
            break;
        }
	    case SUB_CS2L_UPDATE_SMS_PID:
        {
            OnUpdateSMSPid(pData, wDataSize);
            break;
        }
        default:
            break;
	}
}

CT_VOID CLoginServerThread::OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_P2LS_BIND:
	{
		if (wDataSize != sizeof(PS_BindData))
		{
			return;
		}

		PS_BindData* pBindData = (PS_BindData*)pData;
		CServerMgr::get_instance().BindProxyServer(pSocket, pBindData);
		//CNetModule::getSingleton().CloseSocket(pSocket);
	}
	break;
	case  SUB_P2LS_USER_LOGOUT:
	{
		OnUserLogout(pData, wDataSize);
	}
	break;
	default:
		break;
	}
}

CT_VOID CLoginServerThread::OnUserLoginMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS_ACCOUNTS_LOGIN:
		OnAccountsLogin(pData, wDataSize, pSocket);
		break;
	case SUB_CS_ACCOUNTS_REGISTER:
		//OnAccountsRegister(pData, wDataSize, pSocket);
		break;
	case SUB_CS_ACCOUNTS_LOGIN_REGISTER:
		OnAccountsLoginOrRegister(pData, wDataSize, pSocket);
		break;
	case SUB_CS_ACCUNNTS_SIGN_IN:
		//OnUserSignIn(pData, wDataSize, pSocket);
		break;
	case SUB_CS_PULL_MAIL:
		OnUserPullMail(pData, wDataSize, pSocket);
		break;
	case SUB_CS_READ_MAIL:
		OnUserReadMail(pData, wDataSize, pSocket);
		break;
	case SUB_CS_DELETE_MAIL:
		OnUserDelMail(pData, wDataSize, pSocket);
		break;
    case SUB_CS_QUERY_BENEFIT:
        OnUserQueryBenefit(pData, wDataSize, pSocket);
        break;
	case SUB_CS_GET_BENEFIT:
		OnUserGetBenefit(pData, wDataSize, pSocket);
		break;
	case SUB_CS_EXCHANGE_SCORE:
		//OnUserExchangeScore(pData, wDataSize, pSocket);
		break;
	case SUB_CS_BIND_PROMOTER:
		//OnUserBindPromoter(pData, wDataSize, pSocket);
		break;
	case SUB_CS_BIND_MOBILE:
		OnUserBindMobile(pData, wDataSize, pSocket);
		break;
	case SUB_CS_SET_BANK_PWD:
		OnUserSetBankPwd(pData, wDataSize, pSocket);
		break;
	case SUB_CS_MODIFY_BANK_PWD:
		OnUserModifyBankPwd(pData, wDataSize, pSocket);
		break;
	case SUB_CS_SAVE_SCORE_TO_BANK:
		OnUserSaveBankScore(pData, wDataSize, pSocket);
		break;
	case SUB_CS_TAKE_SCORE_FROM_BANK:
		OnUserTakeBankScore(pData, wDataSize, pSocket);
		break;
	case SUB_CS_PAY_SCORE_BY_APPLY:
		OnUserPayScoreByApple(pData, wDataSize, pSocket);
		break;
	case SUB_CS_GET_VERIFICATION_CODE:
		OnUserGetVerifyCode(pData, wDataSize, pSocket);
		break;
	case SUB_CS_RESET_PWD:
		OnUserResetPwd(pData, wDataSize, pSocket);
		break;
	case SUB_CS_MODIFY_PERSONAL_INFO:
		OnUserModifyPersonalInfo(pData, wDataSize, pSocket);
		break;
	case SUB_CS_BIND_ALIPAY_ACCOUNT:
		OnUserBindAliPay(pData, wDataSize, pSocket);
		break;
	case SUB_CS_BIND_BANK_ACCOUNT:
		OnUserBindBankCard(pData, wDataSize, pSocket);
		break;
	case SUB_CS_EXCHANGE_SCORE_TO_RMB:
		OnUserExchangeScoreToRmb(pData, wDataSize, pSocket);
		break;
	case SUB_CS_EXCHANGE_RMB_RECORD:
		OnUserExchangeScoreRecord(pData, wDataSize, pSocket);
		break;
	case SUB_CS_UNRED_EXCHANGE:
		OnUserUnRedExchange(pData, wDataSize, pSocket);
		break;
	case SUB_CS_REPORT_USER:
        OnUserReportUser(pData, wDataSize, pSocket);
		break;
	case SUB_CS_QUERY_RECHARGE_RECORD:
		OnUserQueryRechargeRecord(pData, wDataSize, pSocket);
		break;
		
	default:
		break;
	}
}

CT_VOID CLoginServerThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_LoginServer_Info registerLogin;
		registerLogin.dwServerID = CServerCfg::m_nServerID;
		registerLogin.cbMainServer = CServerCfg::m_MainLoginServer;
		_snprintf_info(registerLogin.szServerIP, sizeof(registerLogin.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerLogin.szServerPubIP, sizeof(registerLogin.szServerPubIP), "%s", CServerCfg::m_PublicAddress);
		_snprintf_info(registerLogin.szServerName, sizeof(registerLogin.szServerName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_LCS_MAIN, SUB_L2CS_REGISTER, &registerLogin, sizeof(registerLogin));

		return;
	}
	LOG(WARNING) << "go center server register failed, center net is not running...";
}


CT_VOID CLoginServerThread::AcrossTheDayEvent()
{
	CT_INT64 nTimeNow = time(NULL);
	CT_BOOL bAcrossDay = isAcrossTheDay(m_iTodayTime, nTimeNow);
	if (bAcrossDay)
	{
		m_iTodayTime = nTimeNow;

		LOG(WARNING) << "login server is across day.";
		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		//清空今日登录IP表
		do 
		{
			acl::query query;
			query.create_sql("DELETE FROM record_register_ip");

			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "delete record_register_ip error" << ", errno: " << db->get_errno() << ", error: " << db->get_error();
				break;
			}
		} while (0);
		
		pool->put(db);
	}
}

CT_VOID CLoginServerThread::OnAccountsLogin(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_LoginDataEx))
	{
		return;
	}

	MSG_CS_LoginDataEx* pAccounts = (MSG_CS_LoginDataEx*)pData;
	if (pAccounts == NULL)
	{
		return;
	}

	//检查版本号

	/*auto it = m_mapGameVersion.find(pAccounts->wGameID);
	if (it == m_mapGameVersion.end())
	{
	SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_VERSION_TOO_LOW);
	return;
	}
	stGameVersion& verion = it->second;
	std::string strClientVersion(pAccounts->chVersion);
	stGameVersion clientVersion;
	GetGameVersion(strClientVersion, clientVersion);

	if (clientVersion.wMainVer != verion.wMainVer || clientVersion.wSubVer != verion.wSubVer)
	{
	SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_VERSION_TOO_LOW);
	return;
	}*/

	static UserAccountsBaseData accountData;
	memset(&accountData, 0, sizeof(UserAccountsBaseData));
    accountData.cbPlatformID = pAccounts->cbPlatformID;

	//LOG(INFO) << "login start time: " << Utility::GetTick();
	CT_BYTE byRet = 0;
	//先检查缓存中有没有数据
	CT_BOOL bInRedis = GetAccountInfo(pAccounts->cbPlatformID, pAccounts->szAccount, accountData);
	if (!bInRedis)
	{
		//LOG(INFO) << "login in db";
		acl::db_pool* accountPool = GetAccountdbPool();
		if (accountPool == NULL)
		{
			LOG(WARNING) << "get account db pool fail.";
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_NETBREAK);
			return;
		}

		acl::db_handle* accountdb = accountPool->peek_open();
		if (accountdb == NULL)
		{
			LOG(WARNING) << "get account db handle fail.";
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_NETBREAK);
			return;
		}

		byRet = GoDBAccountLogin(accountdb, pAccounts, accountData);
		accountPool->put(accountdb);

		if (byRet == 0)
		{
			QueryUserEarnScore(accountData.dwUserID);
			m_vecTask.clear();
			QueryTaskInfo(accountData.dwUserID);
			SetUserTaskInfo(accountData.dwUserID);
			QueryUserTodayRankInfo(accountData.dwUserID);
			m_benefitData.cbVip2 = accountData.cbVipLevel2;
			QueryBenefitInfo(accountData.dwUserID);
			SetUserBenefitInfo(accountData.dwUserID);
            QueryGamePlayData(accountData.dwUserID);
			//QueryGoodCardInfo(accountData);
		}
		/*
		if (m_cbScoreMode)
		{
		QueryBenefitInfo(accountData.dwUserID);
		SetUserBenefitInfo(accountData.dwUserID);

		QueryScoreInfo(accountData.dwUserID);
		SetUserScoreInfo(accountData.dwUserID);

		m_vecTask.clear();
		QueryTaskInfo(accountData.dwUserID);
		SetUserTaskInfo(accountData.dwUserID);
		}
		*/

		//QueryUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
		//SetUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
	}
	else
	{
		//检测密码
		if (strcmp(accountData.szPassword, pAccounts->szPassword) != 0)
		{
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_PASSWORD_ERCOR);
			LOG(WARNING) << "user login fail, user name: " << pAccounts->szAccount << ", user pwd: " << pAccounts->szPassword \
				<< ", redis pwd: " << accountData.szPassword << ", client ip: " << pAccounts->szClientIP;
			return;
		}

		//检测是否被封号
		if (accountData.cbStatus == en_Account_SEAL)
		{
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_SEAL_ACCOUNTS);
			return;
		}
		//LOG(WARNING) << "login in redis.";
	}

	if (0 == byRet)
	{
        accountData.dwClientChannelID = pAccounts->dwChannelID;
        //accountData.cbPlatformID = pAccounts->cbPlatformID;

		std::map<acl::string, acl::string> mapOnlineInfo;
		/*CT_BOOL bInGame =*/ CheckUserOtherGameOnline(accountData.dwUserID, 0, mapOnlineInfo);
		/*if (!bInGame)
		{
			SetUserGoodCardInfo(accountData);
		}*/
		/*if (bInOtherGame)
		{
		//注册失败
		SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_IN_OTHER_GAME);
		return;
		}*/

		//std::string strUUID("NoUUID") = CNetModule::getSingleton().CreateGuidString();
		//_snprintf_info(accountData.szUUID, sizeof(accountData.szUUID), "%s", "NoUUID");
		//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pAccounts->szLocation
		_snprintf_info(accountData.szProvince, sizeof(accountData.szProvince), "%s", pAccounts->szProvince);
		if (strlen(pAccounts->szLocation) != 0)
		{
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pAccounts->szLocation);
		}
		else
		{
			//根据IP获取地区
			std::string strCountry;
			std::string strLocation;
			m_pIpFinder.GetAddressByIp(pAccounts->szClientIP, strCountry, strLocation);
#ifdef _OS_WIN32_CODE
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Ansi2Utf8(strCountry).c_str());
#else
			//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Gbk2Utf(strCountry).c_str());

			if (strCountry.length() >= 1024)
			{
				strCountry = strCountry.substr(0, 1023);
			}
			CT_CHAR szLocation[1024] = { 0 };
			Utility::Gbk2Utf(strCountry.c_str(), strCountry.length(), szLocation, 1024);
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", szLocation);
			accountData.szLocation[USER_LOCATE_LEN - 1] = '\0';
#endif // _OS_WIN32_CODE
		}

		_snprintf_info(accountData.szIP, sizeof(accountData.szIP), "%s", pAccounts->szClientIP);
		_snprintf_info(accountData.szMachineType, sizeof(accountData.szMachineType), "%s", pAccounts->szMachineType);
		SetProxyServerInfo(pSocket, pAccounts->uClientSock, accountData.dwUserID, pAccounts->szMachineSerial);
		//SetUserGoodCardInfo(accountData);
		//SetUserSession(accountData.dwUserID, strUUID);
		if (!bInRedis)
		{
			SetUserAccountInfo(pAccounts->szAccount, accountData);
			SetAccountUserID(pAccounts->cbPlatformID, pAccounts->szAccount, accountData.dwUserID);
		}
		else
		{
			SetUserLocationAndIp(accountData.dwUserID, accountData.szLocation, accountData.szIP);
			SetAccountRelatedPersist(pAccounts->cbPlatformID, pAccounts->szAccount, accountData.dwUserID);
		}

		SendUserAllSucMsg(pSocket, pAccounts, accountData, mapOnlineInfo);
		//LOG(INFO) << "login end time: " << Utility::GetTick();
		return;
	}

	//注册失败
	SendLoginFail(pSocket, pAccounts->uClientSock, byRet);
}

CT_VOID CLoginServerThread::OnAccountsRegister(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_LoginDataEx))
	{
		return;
	}

	MSG_CS_LoginDataEx* pRegister = (MSG_CS_LoginDataEx*)pData;
	if (pRegister == NULL)
	{
		return;
	}

	//判断帐号密码是否为空
	if (strlen(pRegister->szAccount) == 0 || strlen(pRegister->szPassword) == 0)
	{
		SendLoginFail(pSocket, pRegister->uClientSock, REGISTER_CHAR_EMPTY);
		return;
	}

	//检测验证码
	CT_DWORD dwVerifyCode = 0;
	CT_BOOL	bGetCode = GetMobileVerifyCode(pRegister->cbPlatformID, pRegister->szAccount, dwVerifyCode);
	if (!bGetCode || dwVerifyCode != pRegister->dwVerificationCode)
	{
		SendLoginFail(pSocket, pRegister->uClientSock, REGISTER_VERIFYCODE_ERROR);
		return;
	}

	static UserAccountsBaseData accountData;
	memset(&accountData, 0, sizeof(UserAccountsBaseData));
	accountData.cbPlatformID = pRegister->cbPlatformID;

	CT_BYTE byRet = 0;
	acl::db_pool* accountPool = GetAccountdbPool();
	if (accountPool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		SendLoginFail(pSocket, pRegister->uClientSock, LOGIN_NETBREAK);
		return;
	}

	acl::db_handle* accountdb = accountPool->peek_open();
	if (accountdb == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		SendLoginFail(pSocket, pRegister->uClientSock, LOGIN_NETBREAK);
		return;
	}
	byRet = GoDBAccountRegister(accountdb, pRegister, accountData);
	accountPool->put(accountdb);

	if (byRet == 0 )
	{
		QueryUserEarnScore(accountData.dwUserID);
		QueryUserTodayRankInfo(accountData.dwUserID);
	}

	//QueryUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
	//SetUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
	//m_vecTask.clear();
	//QueryTaskInfo(accountData.dwUserID);
	//SetUserTaskInfo(accountData.dwUserID);

	if (0 == byRet)
	{
        accountData.dwClientChannelID = pRegister->dwChannelID;

		std::map<acl::string, acl::string> mapOnlineInfo;
		CheckUserOtherGameOnline(accountData.dwUserID, 0, mapOnlineInfo);

		//std::string strUUID = CNetModule::getSingleton().CreateGuidString();
		//_snprintf_info(accountData.szUUID, sizeof(accountData.szUUID), "%s", "NoUUID");

		if (strlen(pRegister->szLocation) != 0)
		{
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pRegister->szLocation);
            _snprintf_info(accountData.szProvince, sizeof(accountData.szProvince), "%s", pRegister->szProvince);
		}
		else
		{
			//根据IP获取地区
			std::string strCountry;
			std::string strLocation;
			m_pIpFinder.GetAddressByIp(pRegister->szClientIP, strCountry, strLocation);
			//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pRegister->szLocation);
#ifdef _OS_WIN32_CODE
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Ansi2Utf8(strCountry).c_str());
#else
			//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Gbk2Utf(strCountry).c_str());
			CT_CHAR szLocation[1024] = { 0 };
			if (strCountry.length() >= 1024)
			{
				strCountry = strCountry.substr(0, 1023);
			}
			Utility::Gbk2Utf(strCountry.c_str(), strCountry.length(), szLocation, 1024);
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", szLocation);
			accountData.szLocation[USER_LOCATE_LEN - 1] = '\0';
#endif // _OS_WIN32_CODE
		}

		_snprintf_info(accountData.szIP, sizeof(accountData.szIP), "%s", pRegister->szClientIP);
		_snprintf_info(accountData.szMachineType, sizeof(accountData.szMachineType), "%s", pRegister->szMachineType);
		SetProxyServerInfo(pSocket, pRegister->uClientSock, accountData.dwUserID, pRegister->szMachineSerial);
		//SetUserSession(accountData.dwUserID, strUUID);
		SetUserAccountInfo(pRegister->szAccount, accountData);
		SetAccountUserID(pRegister->cbPlatformID, pRegister->szAccount, accountData.dwUserID);

		SendUserAllSucMsg(pSocket, pRegister, accountData, mapOnlineInfo);

		//删除验证码
		DeleteMobileVerifyCode(pRegister->cbPlatformID, pRegister->szAccount);
		return;
	}

	//注册失败
	SendLoginFail(pSocket, pRegister->uClientSock, byRet);
}

CT_VOID CLoginServerThread::OnAccountsLoginOrRegister(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_LoginDataEx))
	{
		return;
	}

	MSG_CS_LoginDataEx* pAccounts = (MSG_CS_LoginDataEx*)pData;
	if (pAccounts == NULL)
	{
		return;
	}

	//LOG(WARNING) << "user province:" << pAccounts->szProvince;

	//检查版本号

	/*auto it = m_mapGameVersion.find(pAccounts->wGameID);
	if (it == m_mapGameVersion.end())
	{
		SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_VERSION_TOO_LOW);
		return;
	}
	stGameVersion& verion = it->second;
	std::string strClientVersion(pAccounts->chVersion);
	stGameVersion clientVersion;
	GetGameVersion(strClientVersion, clientVersion);

	if (clientVersion.wMainVer != verion.wMainVer || clientVersion.wSubVer != verion.wSubVer)
	{
		SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_VERSION_TOO_LOW);
		return;
	}*/
	
	static UserAccountsBaseData accountData;
	memset(&accountData, 0, sizeof(UserAccountsBaseData));
    accountData.cbPlatformID = pAccounts->cbPlatformID;

	//LOG(INFO) << "login start time: " << Utility::GetTick();
	CT_BYTE byRet = 0;
	//先检查缓存中有没有数据
	CT_BOOL bInRedis = GetAccountInfo(pAccounts->cbPlatformID, pAccounts->szAccount, accountData);
	if (!bInRedis)
	{
		//LOG(INFO) << "login in db";
		acl::db_pool* accountPool = GetAccountdbPool();
		if (accountPool == NULL)
		{
			LOG(WARNING) << "get account db pool fail.";
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_NETBREAK);
			return;
		}

		acl::db_handle* accountdb = accountPool->peek_open();
		if (accountdb == NULL)
		{
			LOG(WARNING) << "get account db handle fail.";
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_NETBREAK);
			return;
		}

		byRet = GoDBAccountLoginOrRegister(accountdb, pAccounts, accountData);
		accountPool->put(accountdb);

		if (byRet == 0)
		{
		    //如果是群组成员，则到中心注册本会员
		    if (accountData.cbNewAccount != 0 && pAccounts->dwGroupUserID != 0)
            {
                GoCenterAddGroupUser(accountData, pAccounts->dwGroupUserID);
            }
			QueryUserEarnScore(accountData.dwUserID);
			m_vecTask.clear();
			QueryTaskInfo(accountData.dwUserID);
			SetUserTaskInfo(accountData.dwUserID);
			QueryUserTodayRankInfo(accountData.dwUserID);
			m_benefitData.cbVip2 = accountData.cbVipLevel2;
			QueryBenefitInfo(accountData.dwUserID);
			SetUserBenefitInfo(accountData.dwUserID);
            QueryGamePlayData(accountData.dwUserID);

            //
			//QueryGoodCardInfo(accountData);
		}

		/*
		if (m_cbScoreMode)
		{
			QueryBenefitInfo(accountData.dwUserID);
			SetUserBenefitInfo(accountData.dwUserID);

			QueryScoreInfo(accountData.dwUserID);
			SetUserScoreInfo(accountData.dwUserID);

			m_vecTask.clear();
			QueryTaskInfo(accountData.dwUserID);
			SetUserTaskInfo(accountData.dwUserID);
		}
		*/

		//QueryUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
		//SetUserGameRound(accountData.dwUserID, m_vecGameRoundInfo);
	}
	else
	{
		//检测是否被封号
		if (accountData.cbStatus == en_Account_SEAL)
		{
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_SEAL_ACCOUNTS);
			return;
		}
		//检测是否已经绑定账号(防止通过手机号从redis走游客登陆)
		if (strlen(accountData.szMobileNum) != 0)
        {
            byRet = LOGIN_PASSWORD_ERCOR;
        }
        //LOG(WARNING) << "login in redis.";
	}

	if (0 == byRet)
	{
        accountData.dwClientChannelID = pAccounts->dwChannelID;
        //accountData.cbPlatformID = pAccounts->cbPlatformID;

		std::map<acl::string, acl::string> mapOnlineInfo;
		/*CT_BOOL bInGame = */CheckUserOtherGameOnline(accountData.dwUserID, 0, mapOnlineInfo);
	    /*if (!bInGame)
		{
			SetUserGoodCardInfo(accountData);
		}*/
		/*if (bInOtherGame)
		{
			//注册失败
			SendLoginFail(pSocket, pAccounts->uClientSock, LOGIN_IN_OTHER_GAME);
			return;
		}*/

		//std::string strUUID = CNetModule::getSingleton().CreateGuidString();
		//_snprintf_info(accountData.szUUID, sizeof(accountData.szUUID), "%s", "NoUUID");
		//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pAccounts->szLocation
		_snprintf_info(accountData.szProvince, sizeof(accountData.szProvince), "%s", pAccounts->szProvince);
		if (strlen(pAccounts->szLocation) != 0)
		{
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", pAccounts->szLocation);
		}
		else
		{
			//根据IP获取地区
			std::string strCountry;
			std::string strLocation;
			m_pIpFinder.GetAddressByIp(pAccounts->szClientIP, strCountry, strLocation);
#ifdef _OS_WIN32_CODE
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Ansi2Utf8(strCountry).c_str());
#else
			//_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", Utility::Gbk2Utf(strCountry).c_str());
			if (strCountry.length() > 1024)
			{
				strCountry = strCountry.substr(0, 1023);
			}
			CT_CHAR szLocation[1024] = { 0 };
			Utility::Gbk2Utf(strCountry.c_str(), strCountry.length(), szLocation, 1024);
			_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", szLocation);
			accountData.szLocation[USER_LOCATE_LEN - 1] = '\0';
#endif // _OS_WIN32_CODE
		}
		_snprintf_info(accountData.szIP, sizeof(accountData.szIP), "%s", pAccounts->szClientIP);
		_snprintf_info(accountData.szMachineType, sizeof(accountData.szMachineType), "%s", pAccounts->szMachineType);
		SetProxyServerInfo(pSocket, pAccounts->uClientSock, accountData.dwUserID, pAccounts->szMachineSerial);
        //SetUserGoodCardInfo(accountData);
		//SetUserSession(accountData.dwUserID, strUUID);
		if (!bInRedis)
		{
			SetUserAccountInfo(pAccounts->szAccount, accountData);
			SetAccountUserID(pAccounts->cbPlatformID, pAccounts->szAccount, accountData.dwUserID);
		}
		else
		{
			SetUserLocationAndIp(accountData.dwUserID, accountData.szLocation, accountData.szIP);
			SetAccountRelatedPersist(pAccounts->cbPlatformID, pAccounts->szAccount, accountData.dwUserID);
		}
	
		SendUserAllSucMsg(pSocket, pAccounts, accountData, mapOnlineInfo);
		//LOG(INFO) << "login end time: " << Utility::GetTick();
		return;
	}

	//注册失败
	if (byRet == LOGIN_PASSWORD_ERCOR)
	{
		MSG_SC_BindMobile_Already bindMobileAlready;
		bindMobileAlready.dwMainID = MSG_LOGIN_MAIN;
		bindMobileAlready.dwSubID = SUB_SC_BIND_MOBILE_ALREADY;
		bindMobileAlready.uValue1 = pAccounts->uClientSock;
		_snprintf_info(bindMobileAlready.szMobileNum, sizeof(bindMobileAlready.szMobileNum), "%s", accountData.szMobileNum);

		SendMsg(pSocket, &bindMobileAlready, sizeof(bindMobileAlready));
	}
	else
	{
		SendLoginFail(pSocket, pAccounts->uClientSock, byRet);
	}
}

CT_VOID CLoginServerThread::OnUserSignIn(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_SignInEx))
	{
		return;
	}

	MSG_CS_SignInEx* pSignIn = (MSG_CS_SignInEx*)pData;
	if (pSignIn == NULL)
	{
		return;
	}

	/*if (IsExistUserSession(pSignIn->dwUserID, pSignIn->szUUID) == false)
	{
		LOG(WARNING) << "user sign in, but can not find user session!";
		return;
	}*/

	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}
	
	QuerySignInInfoIndb(db, pSignIn->dwUserID, m_signInData);
	
	//向前端发送结果
	MSG_SC_SignIn_Result signInResult;
	signInResult.dwMainID = MSG_LOGIN_MAIN;
	signInResult.dwSubID = SUB_SC_SIGN_IN_RESULT;
	signInResult.uValue1 = pSignIn->uClientSock;

	//更新帐号库的钻石
	if (m_signInData.bNewDate)
	{
		CT_WORD signinDay = m_signInData.wSignInDays >= 7 ? 7 : m_signInData.wSignInDays+1;
		auto it = m_mapsignInConfig.find(signinDay);

		if (it != m_mapsignInConfig.end())
		{
			CT_DWORD dwUserGem = GetUserGem(pSignIn->dwUserID);
			CT_LONGLONG llUserScore = GetUserScore(pSignIn->dwUserID);
			CT_BYTE cbVipLevel = GetUserVipLevel(pSignIn->dwUserID);

			CT_LONGLONG llRewardScore = it->second.dwRewardScore;
			if (cbVipLevel != 0)
			{
				auto it = m_mapVipReward.find(cbVipLevel);
				if (it != m_mapVipReward.end())
				{
					llRewardScore += it->second.dwSignInRewardScore;
				}
			}

			AddUserGemAndScore(pSocket, pSignIn->dwUserID, it->second.wRewardGem, llRewardScore);
			UpdateSignInRecord(db, pSignIn->dwUserID/*, it->second.wRewardGem, llRewardScore*/);
			InsertScoreChangeRecord(pSignIn->dwUserID, dwUserGem, llUserScore, it->second.wRewardGem, llRewardScore, SIGN_IN_REWARD);

			signInResult.cbResult = 0;
			signInResult.wSignInDays = m_signInData.wSignInDays + 1;
			signInResult.wRewardGem = it->second.wRewardGem;
			signInResult.dwRewardScore = (CT_DWORD)llRewardScore;
		}
		else
		{
			signInResult.cbResult = 1;
			signInResult.wSignInDays = m_signInData.wSignInDays;
			signInResult.wRewardGem = 0;
			signInResult.dwRewardScore = 0;
		}	
	}
	else
	{
		signInResult.cbResult = 1;
		signInResult.wSignInDays = m_signInData.wSignInDays;
		signInResult.wRewardGem = 0;
		signInResult.dwRewardScore = 0;
	}
	m_signInData.Reset();
	SendMsg(pSocket, &signInResult, sizeof(signInResult));

	pool->put(db);

}

CT_VOID CLoginServerThread::OnUserQueryBenefit(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
    if (wDataSize != sizeof(MSG_CS_QueryBenefit))
    {
        return;
    }

    MSG_CS_QueryBenefit* pBenefit = (MSG_CS_QueryBenefit*)pData;


    MSG_SC_QueryBenefit queryBenefit;
    queryBenefit.dwMainID = MSG_LOGIN_MAIN;
    queryBenefit.dwSubID = SUB_SC_QUERY_BENEFIT;
    queryBenefit.dwValue2 = pBenefit->dwUserID;

    bool bResult = GetUserBenefitInfo(pBenefit->dwUserID);
    if (!bResult)
    {
        queryBenefit.cbBenefitTotalCount = 0;
        queryBenefit.cbBenefitLeftCount = 0;
        queryBenefit.dRewardScore = 0.0f;
		SendMsgByUserID(pSocket, &queryBenefit, sizeof(queryBenefit));
		return;
    }

	tagBenefitReward* pBenefitReward = GetBenefitReward(m_benefitData.cbVip2);
    if (pBenefitReward == NULL)
	{
		queryBenefit.cbBenefitTotalCount = 0;
		queryBenefit.cbBenefitLeftCount = 0;
		queryBenefit.dRewardScore = 0.0f;
		SendMsgByUserID(pSocket, &queryBenefit, sizeof(queryBenefit));
		return;
	}

    CT_LONGLONG llUserBankScore = GetUserBankScore(pBenefit->dwUserID);
    CT_LONGLONG llUserScore = GetUserScore(pBenefit->dwUserID);

	queryBenefit.cbBenefitTotalCount = pBenefitReward->cbRewardCount;
	queryBenefit.dLessScore = pBenefitReward->dwLessScore*TO_DOUBLE;
	queryBenefit.cbBenefitLeftCount = pBenefitReward->cbRewardCount - m_benefitData.cbCurrCount;
	queryBenefit.dRewardScore = pBenefitReward->dwRewardScore*TO_DOUBLE;

    if (llUserBankScore + llUserScore < pBenefitReward->dwLessScore && m_benefitData.cbCurrCount < pBenefitReward->cbRewardCount)
	{
		//queryBenefit.dRewardScore = pBenefitReward->dwRewardScore*TO_DOUBLE;
		queryBenefit.cbCanGetBenefit = 1;
	}
    else
	{
		//queryBenefit.dRewardScore = 0.0f;
        queryBenefit.cbCanGetBenefit = 0;
	}


	SendMsgByUserID(pSocket, &queryBenefit, sizeof(queryBenefit));
}

CT_VOID CLoginServerThread::OnUserGetBenefit(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_GetBenefitEx))
	{
		return;
	}

	MSG_CS_GetBenefitEx* pGetBenefit = (MSG_CS_GetBenefitEx*)pData;
	if (pGetBenefit == NULL)
	{
		return;
	}

	MSG_SC_GetBenefit_Result benefitResult;
	benefitResult.dwMainID = MSG_LOGIN_MAIN;
	benefitResult.dwSubID = SUB_SC_GET_BENEFIT_RESULT;
	benefitResult.uValue1 = pGetBenefit->uClientSock;

	CT_BOOL bBenefitInfo = GetUserBenefitInfo(pGetBenefit->dwUserID);
	if (!bBenefitInfo)
	{
		benefitResult.cbResult = 1;
	}
	else
	{
		tagBenefitReward* pBenefitReward = GetBenefitReward(m_benefitData.cbVip2);
		if (pBenefitReward != NULL)
		{
			//检查是否同一天
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			bool bAcrossDay = isAcrossTheDay(m_benefitData.dwLastTime, dwNow);

			if (bAcrossDay)
			{
				m_benefitData.cbCurrCount = 0;
			}

			if (m_benefitData.cbCurrCount >= pBenefitReward->cbRewardCount)
			{
				benefitResult.cbResult = 1;
			}
			else
			{
				//检查是否金币少于领取范围
				CT_LONGLONG llScore = GetUserScore(pGetBenefit->dwUserID);
				if (llScore >= pBenefitReward->dwLessScore)
				{
					benefitResult.cbResult = 2;
				}
				else
				{
					//正式领取
					m_benefitData.cbCurrCount += 1;
					m_benefitData.dwLastTime = dwNow;

					//写入db
					acl::db_pool* pool = GetGamedbPool();
					if (pool == NULL)
					{
						LOG(WARNING) << "get game db pool fail.";
						return;
					}

					acl::db_handle* db = pool->peek_open();
					if (db == NULL)
					{
						LOG(WARNING) << "get game db handle fail.";
						return;
					}

					InsertBenefitRecord(db, pGetBenefit->dwUserID, pBenefitReward->dwRewardScore);
					pool->put(db);

                    InsertScoreChangeRecord(pGetBenefit->dwUserID, 0, llScore,  0, pBenefitReward->dwRewardScore, BENEFIT_REWARD);

                    AddUserGemAndScore(pSocket, pGetBenefit->dwUserID, 0, pBenefitReward->dwRewardScore, true);

                    //设置缓存
                    SetUserBenefitInfo(pGetBenefit->dwUserID);

					benefitResult.cbResult = 0;
				}
			}
		}
		else
		{
			benefitResult.cbResult = 1;
		}
	}

	m_benefitData.Reset();
	SendMsg(pSocket, &benefitResult, sizeof(benefitResult));
}

CT_VOID CLoginServerThread::OnUserExchangeScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_ExchangeScoreEx))
	{
		return;
	}

	MSG_CS_ExchangeScoreEx* pExchangeScore = (MSG_CS_ExchangeScoreEx*)pData;
	if (pExchangeScore == NULL)
	{
		return;
	}

	MSG_SC_ExchangeScore_Result exchangeResult;
	exchangeResult.dwMainID = MSG_LOGIN_MAIN;
	exchangeResult.dwSubID = SUB_SC_EXCHANGE_SCORE_RESULT;
	exchangeResult.uValue1 = pExchangeScore->uClientSock;

	CT_DWORD dwGem = GetUserGem(pExchangeScore->dwUserID);
	if (dwGem < pExchangeScore->wGem)
	{
		exchangeResult.cbResult = 1;
		SendMsg(pSocket, &exchangeResult, sizeof(exchangeResult));
		return;
	}

	CT_LONGLONG llScore =  GetUserScore(pExchangeScore->dwUserID);

	auto it = m_mapExchageScoreCfg.find(pExchangeScore->wGem);
	if (it == m_mapExchageScoreCfg.end())
	{
		exchangeResult.cbResult = 2;
		SendMsg(pSocket, &exchangeResult, sizeof(exchangeResult));
		return;
	}

	AddUserGemAndScore(pSocket, pExchangeScore->dwUserID, -pExchangeScore->wGem, it->second.dwExchangeScore + it->second.dwExtraPresentScore);
	exchangeResult.cbResult = 0;
	exchangeResult.wConsumeGem = pExchangeScore->wGem;
	exchangeResult.llAddScore = it->second.dwExchangeScore + it->second.dwExtraPresentScore;

	//写入db
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}
	InsertScoreChangeRecord(pExchangeScore->dwUserID, dwGem, llScore, -pExchangeScore->wGem, it->second.dwExchangeScore + it->second.dwExtraPresentScore, EXCHANGE_SCORE);
	pool->put(db);


	SendMsg(pSocket, &exchangeResult, sizeof(exchangeResult));

}

CT_VOID CLoginServerThread::OnUserBindPromoter(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_Promoter))
	{
		return;
	}

	MSG_CS_Bind_Promoter* pBindPromoter = (MSG_CS_Bind_Promoter*)pData;
	
	CT_DWORD dwPromoterID = GetUserBindPromoterID(pBindPromoter->dwUserID);

	MSG_SC_Bind_Promoter bindPromoterResult;
	bindPromoterResult.dwMainID = MSG_LOGIN_MAIN;
	bindPromoterResult.dwSubID = SUB_SC_BIND_PROMOTER;
	bindPromoterResult.dwValue2 = pBindPromoter->dwUserID;
	if (dwPromoterID != 0)
	{
		bindPromoterResult.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindPromoterResult, sizeof(bindPromoterResult));
		return;
	}

	//到DB绑定代理ID
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		bindPromoterResult.cbResult = 2;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindPromoterResult, sizeof(bindPromoterResult));
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		bindPromoterResult.cbResult = 2;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindPromoterResult, sizeof(bindPromoterResult));
		return;
	}
	
	CT_BYTE cbResult = BindUserPromoterID(db, pBindPromoter->dwUserID, pBindPromoter->dwPromoterID);
	bindPromoterResult.cbResult = cbResult;
	bindPromoterResult.wRewardGem = 20;
	bindPromoterResult.dwPromoterID = pBindPromoter->dwPromoterID;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindPromoterResult, sizeof(bindPromoterResult));
	pool->put(db);
	//给玩家添加钻石
	if (cbResult == 0)
	{
		SetUserBindPromoterID(pBindPromoter->dwUserID, pBindPromoter->dwPromoterID);
		AddUserGemAndScore(pSocket, pBindPromoter->dwUserID, 20, 0, true);
	}
}

CT_VOID CLoginServerThread::OnUserBindMobile(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_Mobile))
	{
		return;
	}

	MSG_CS_Bind_Mobile* pBindAccount = (MSG_CS_Bind_Mobile*)pData;
	if (pBindAccount == NULL)
	{
		return;
	}

	MSG_SC_Bind_Mobile bindResult;
	bindResult.dwMainID = MSG_LOGIN_MAIN;
	bindResult.dwSubID = SUB_SC_BIND_MOBILE;
	bindResult.dwValue2 = pBindAccount->dwUserID;

	//检测此帐号是否已经绑定帐号
	acl::string strMobileNum;
	GetUserMobileNum(pBindAccount->dwUserID, strMobileNum);
	if (!strMobileNum.empty())
	{
		bindResult.cbResult = 2;
		SendMsgByUserID(pSocket, &bindResult, sizeof(bindResult));
		return;
	}

	//判断密码是否为空
	if (strlen(pBindAccount->szPassword) == 0)
	{
		bindResult.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
		return;
	}

	//获取用户平台ID
	CT_BYTE cbPlatformId = GetUserPlatformId(pBindAccount->dwUserID);

	//检测验证码
	CT_DWORD dwVerifyCode = 0;
	CT_BOOL	bGetCode = GetMobileVerifyCode(cbPlatformId, pBindAccount->szMobileNum, dwVerifyCode);
	if (!bGetCode || dwVerifyCode != pBindAccount->dwVerifyCode)
	{
		bindResult.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
		return;
	}

	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		bindResult.cbResult = REGISTER_UNKNOWN;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		bindResult.cbResult = REGISTER_UNKNOWN;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
		return;
	}

	std::string oldAccount;
	acl::string newNickName;
	newNickName.format("%u", pBindAccount->dwUserID);
	std::string newNickNameUtf8(Utility::Ansi2Utf8(newNickName.c_str()));

	//CT_BYTE cbPlatformId = 0;
	CT_BYTE bResult = GoDBBindMobile(db, pBindAccount, oldAccount, newNickName, cbPlatformId);
	if (bResult != 0)
	{
		bindResult.cbResult = 2;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
		pool->put(db);
		return;
	}
	pool->put(db);

	//删除验证码
	DeleteMobileVerifyCode(cbPlatformId, pBindAccount->szMobileNum);

	//修改缓存数据
	SetUserBindMobileInfo(pBindAccount->dwUserID, pBindAccount->szMobileNum, newNickNameUtf8.c_str(),  pBindAccount->szPassword);
	RenameAccountUserID(oldAccount.c_str(), pBindAccount->szMobileNum, cbPlatformId);

	CT_LONGLONG llUserScore = GetUserScore(pBindAccount->dwUserID);
	CT_LONGLONG llUserBank = GetUserBankScore(pBindAccount->dwUserID);

	//赠送金币
	CT_WORD	wChannelID = GetUserChannelID(pBindAccount->dwUserID);
	CT_DWORD dwPresentScore = GetChannelPresentScore(wChannelID, en_BindMobile);

	//插入赠送金币记录
	InsertScoreChangeRecord(pBindAccount->dwUserID, llUserBank, llUserScore, 0, dwPresentScore, PRESENTED_SCORE);

	//赠送金币
	AddUserGemAndScore(pSocket, pBindAccount->dwUserID, 0, dwPresentScore, true);

	bindResult.cbResult = 0;
	_snprintf_info(bindResult.szNickName, sizeof(bindResult.szNickName), "%s", newNickNameUtf8.c_str());
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bindResult, sizeof(bindResult));
}

CT_VOID CLoginServerThread::OnUserPayScoreByApple(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Pay_UserScore_ByApple))
	{
		return;
	}

	if (m_cbOpenApplePay == 0)
	{
		return;
	}

	MSG_CS_Pay_UserScore_ByApple* pPayScore = (MSG_CS_Pay_UserScore_ByApple*)pData;
	AddUserGemAndScore(pSocket, pPayScore->dwUserID, 0, pPayScore->dwAddScore * 1000000, true);
	LOG(WARNING) << "Pay score by apply : " << pPayScore->dwAddScore * 1000000;
}

CT_VOID CLoginServerThread::OnUserPullMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_PullMailEx))
	{
		return;
	}

	MSG_CS_PullMailEx* pPullMail = (MSG_CS_PullMailEx*)pData;
	if (pPullMail == NULL)
	{
		return;
	}

	/*if (IsExistUserSession(pPullMail->dwUserID, pPullMail->szUUID) == false)
	{
		MSG_GameMsgDownHead mailFinish;
		mailFinish.dwMainID = MSG_LOGIN_MAIN;
		mailFinish.dwSubID = SUB_SC_MAIL_INFO_FINISH;
		mailFinish.uValue1 = pPullMail->uClientSock;
		SendMsg(pSocket, &mailFinish, sizeof(mailFinish));
		return;
	}*/

	CT_WORD wChannelID = GetUserChannelID(pPullMail->dwUserID);
	QueryUserMail(pPullMail->dwUserID, wChannelID, m_vecUserMail);
	SendUserMail(pSocket, pPullMail->uClientSock, pPullMail->dwUserID);
}

CT_VOID CLoginServerThread::OnUserReadMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_ReadMailEx))
	{
		return;
	}

	MSG_CS_ReadMailEx* pReadMail = (MSG_CS_ReadMailEx*)pData;
	if (pReadMail == NULL)
	{
		return;
	}

	MSG_SC_ReadMailResult readMailResult;
	memset(&readMailResult, 0, sizeof(readMailResult));
	readMailResult.dwMainID = MSG_LOGIN_MAIN;
	readMailResult.dwSubID = SUB_SC_READ_MAIL_RESULT;
	readMailResult.uValue1 = pReadMail->uClientSock;
	/*if (IsExistUserSession(pReadMail->dwUserID, pReadMail->szUUID) == false)
	{
		LOG(WARNING) << "user read mail, but can not find user session!";

		readMailResult.cbResult = 3;
		SendMsg(pSocket, &readMailResult, sizeof(readMailResult));

		return;
	}*/

	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	MSG_SC_Mail_InfoEx mailInfo;
	memset(&mailInfo, 0, sizeof(mailInfo));
	QueryUserOneMailIndb(db, pReadMail->dwMailID, mailInfo);

	if (mailInfo.dwMailID == 0 || mailInfo.dwUserID == 0 || mailInfo.dwUserID != pReadMail->dwUserID)
	{
		readMailResult.cbResult = 1;
		SendMsg(pSocket, &readMailResult, sizeof(readMailResult));
		pool->put(db);
		return;
	}

	if (mailInfo.cbState == 1)
	{
		readMailResult.cbResult = 2;
		SendMsg(pSocket, &readMailResult, sizeof(readMailResult));
		pool->put(db);
		return;
	}

	//设置邮件已经读取或者删除邮件
	acl::query query;
	if (mailInfo.cbMailType == en_Mail_Delete_AfterRead) //看后即毁
	{
		query.create_sql("insert into mail_del select *, now() from mail where mailid = :mailid")
			.set_format("mailid", "%d", pReadMail->dwMailID);
	}
	else if (mailInfo.cbMailType == en_Mail_Delete_Expire) //看后不毁
	{
		query.create_sql("update mail set state = 1 where mailid = :mailid")
			.set_format("mailid", "%d", pReadMail->dwMailID);
	}

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update mail info fail: " << query.to_string().c_str();
		pool->put(db);
		return;
	}
	db->free_result();

	if (mailInfo.cbMailType == en_Mail_Delete_AfterRead)
	{
		query.reset();
		query.create_sql("delete from mail where mailid = :mailid")
			.set_format("mailid", "%d", pReadMail->dwMailID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "delete mail fail: " << query.to_string().c_str();
			pool->put(db);
			return;
		}
		db->free_result();
	}

	readMailResult.cbResult = 0;
	readMailResult.dwMailID = pReadMail->dwMailID;
	//更新帐号库的钻石或者金币
	if (mailInfo.dScore != 0)
	{
		CT_LONGLONG llUserScore = GetUserScore(pReadMail->dwUserID);
		CT_LONGLONG llUserBankScore = GetUserBankScore(pReadMail->dwUserID);
		CT_LONGLONG llAddUserScore = (CT_LONGLONG)(mailInfo.dScore*TO_LL);
		AddUserGemAndScore(pSocket, pReadMail->dwUserID, 0, llAddUserScore);
		readMailResult.dRewardScore = mailInfo.dScore;
		CT_LONGLONG llUserNewScore = GetUserScore(pReadMail->dwUserID);
		readMailResult.dTotalScore =llUserNewScore*TO_DOUBLE;

		//检测是否不能领取救济金(简单粗暴的做法)
		CT_BYTE cbVip2 = GetUserVip2Level(pReadMail->dwUserID);
		tagBenefitReward* pBenefitReward = GetBenefitReward(cbVip2);
		if (pBenefitReward && llUserNewScore >= pBenefitReward->dwLessScore)
        {
            MSG_SC_HasBenefit hasBenefit;
            hasBenefit.dwMainID = MSG_FRAME_MAIN;
            hasBenefit.dwSubID = SUB_S2C_HAS_BENEFIT;
            hasBenefit.dwValue2 = pReadMail->dwUserID;
            hasBenefit.cbState = 0;
            SendMsgByUserID(pSocket, &hasBenefit, sizeof(MSG_SC_HasBenefit));
        }

		//金币记录
		if (mailInfo.cbScoreType == 0)
		{
			InsertScoreChangeRecord(pReadMail->dwUserID, llUserBankScore, llUserScore, 0, llAddUserScore, MAIL_REWARD, pReadMail->dwMailID);
		}
		else
		{
			InsertScoreChangeRecord(pReadMail->dwUserID, llUserBankScore, llUserScore, 0, llAddUserScore, mailInfo.cbScoreType, pReadMail->dwMailID);
		}
	}

	SendMsg(pSocket, &readMailResult, sizeof(MSG_SC_ReadMailResult));

	pool->put(db);
}

CT_VOID CLoginServerThread::OnUserDelMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_DelMail))
	{
		return;
	}

	MSG_CS_DelMail* pDelMail = (MSG_CS_DelMail*)pData;
	if (pDelMail == NULL)
	{
		return;
	}

	MSG_SC_DeleteMail deleteMail;
	deleteMail.dwMainID = MSG_LOGIN_MAIN;
	deleteMail.dwSubID = SUB_SC_DELETE_MAIL;
	deleteMail.dwValue2 = pDelMail->dwUserID;

	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	MSG_SC_Mail_InfoEx mailInfo;
	memset(&mailInfo, 0, sizeof(mailInfo));
	QueryUserOneMailIndb(db, pDelMail->dwMailID, mailInfo);

	if (mailInfo.dwMailID == 0 || mailInfo.dwUserID == 0 || mailInfo.dwUserID != pDelMail->dwUserID)
	{
		deleteMail.cbResult = 1;
		SendMsg(pSocket, &deleteMail, sizeof(deleteMail));
		pool->put(db);
		return;
	}

	//还没有阅读的邮件不能删除
	if (mailInfo.cbState == 0)
	{
		deleteMail.cbResult = 2;
		SendMsg(pSocket, &deleteMail, sizeof(deleteMail));
		pool->put(db);
		return;
	}

	//把邮件插入删除表
	acl::query query;
	query.create_sql("insert into mail_del select *, now() from mail where mailid = :mailid")
		.set_format("mailid", "%d", pDelMail->dwMailID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update mail info fail: " << db->get_error();
		pool->put(db);
		return;
	}
	db->free_result();

	//删除邮件
	query.reset();
	query.create_sql("delete from mail where mailid = :mailid LIMIT 1")
		.set_format("mailid", "%d", pDelMail->dwMailID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete mail fail: " << db->get_error();
		pool->put(db);
		return;
	}
	db->free_result();

	deleteMail.cbResult = 0;
	deleteMail.dwMailID = pDelMail->dwMailID;
	SendMsgByUserID(pSocket, &deleteMail, sizeof(deleteMail));

	pool->put(db);
}

CT_VOID CLoginServerThread::OnUserSetBankPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Set_BankPwd))
	{
		return;
	}

	MSG_CS_Set_BankPwd* pSetBankPwd = (MSG_CS_Set_BankPwd*)pData;

	std::string strBankPwd;
	if (!GetUserBankPwd(pSetBankPwd->dwUserID, strBankPwd))
	{
		return;
	}


	MSG_SC_Set_BankPwd result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_SET_BANK_PWD;
	result.dwValue2 = pSetBankPwd->dwUserID;
	if (strlen(pSetBankPwd->szBankPwd) == 0)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (strBankPwd.length() != 0)
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	UpdateUserBankPwd(pSetBankPwd->dwUserID, pSetBankPwd->szBankPwd);
	SetUserBankPwd(pSetBankPwd->dwUserID, pSetBankPwd->szBankPwd);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserModifyBankPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Modify_BankPwd))
	{
		return;
	}

	MSG_CS_Modify_BankPwd* pModifyBankPwd = (MSG_CS_Modify_BankPwd*)pData;

	std::string strBankPwd;
	if (!GetUserBankPwd(pModifyBankPwd->dwUserID, strBankPwd))
	{
		return;
	}

	MSG_SC_Modify_BankPwd result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_MODIFY_BANK_PWD;
	result.dwValue2 = pModifyBankPwd->dwUserID;
	if (strBankPwd != std::string(pModifyBankPwd->szOldBankPwd))
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (strlen(pModifyBankPwd->szNewBankPwd) == 0)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	UpdateUserBankPwd(pModifyBankPwd->dwUserID, pModifyBankPwd->szNewBankPwd);
	SetUserBankPwd(pModifyBankPwd->dwUserID, pModifyBankPwd->szNewBankPwd);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserSaveBankScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_SaveScore_To_Bank))
	{
		return;
	}

	MSG_CS_SaveScore_To_Bank* pSaveScoreToBank = (MSG_CS_SaveScore_To_Bank*)pData;

	/*std::string strBankPwd;
	if (!GetUserBankPwd(pSaveScoreToBank->dwUserID, strBankPwd))
	{
		return;
	}*/

	MSG_SC_Save_Score_To_Bank result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_SAVE_SCORE_TO_BANK;
	result.dwValue2 = pSaveScoreToBank->dwUserID;
	/*if (strBankPwd.length() == 0)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}*/

	if (!(pSaveScoreToBank->dSaveScore > 0))
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (CheckUserInGameOnline(pSaveScoreToBank->dwUserID))
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	CT_DOUBLE dSaveScore = Utility::round(pSaveScoreToBank->dSaveScore, 2);
	CT_LONGLONG llSaveScore = (CT_LONGLONG)(dSaveScore * TO_LL);
	CT_LONGLONG llScore = GetUserScore(pSaveScoreToBank->dwUserID);
	CT_LONGLONG llBankScore = GetUserBankScore(pSaveScoreToBank->dwUserID);

	if (llSaveScore < 0)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (llScore < llSaveScore)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	//更新金币
	AddUserGemAndScore(pSocket, pSaveScoreToBank->dwUserID, 0, -llSaveScore, true);

	//更新银行金币
	AddUserBankScore(pSocket, pSaveScoreToBank->dwUserID, llSaveScore);

	//写入金币变化表
	InsertScoreChangeRecord(pSaveScoreToBank->dwUserID, llBankScore, llScore, llSaveScore, -llSaveScore, BANK_SAVE);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserTakeBankScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_TakeScore_From_Bank))
	{
		return;
	}

	MSG_CS_TakeScore_From_Bank* pTakeScore = (MSG_CS_TakeScore_From_Bank*)pData;

	/*std::string strBankPwd;
	if (!GetUserBankPwd(pTakeScore->dwUserID, strBankPwd))
	{
		return;
	}*/

	MSG_SC_Take_Score_From_Bank result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_TAKE_SCORE_FROM_BANK;
	result.dwValue2 = pTakeScore->dwUserID;
	/*if (strBankPwd.length() == 0 || strBankPwd != std::string(pTakeScore->szBankPwd))
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}*/

	if (!(pTakeScore->dTakeScore > 0))
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	CT_LONGLONG llTakeScore = (CT_LONGLONG)(pTakeScore->dTakeScore * TO_LL);
	CT_LONGLONG llBankScore = GetUserBankScore(pTakeScore->dwUserID);
	CT_LONGLONG llScore = GetUserScore(pTakeScore->dwUserID);
	
	if (llTakeScore < 0)
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	
	if (llBankScore < llTakeScore)
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (CheckUserInGameOnline(pTakeScore->dwUserID))
	{
		result.cbResult = 3;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	//更新金币
	AddUserGemAndScore(pSocket, pTakeScore->dwUserID, 0, llTakeScore, true);

	//更新银行金币
	AddUserBankScore(pSocket, pTakeScore->dwUserID, -llTakeScore);

	//写入金币变化表
	InsertScoreChangeRecord(pTakeScore->dwUserID, llBankScore, llScore, -llTakeScore, llTakeScore, BANK_TAKE);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserGetVerifyCode(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Get_VerificationCodeEx))
	{
		return;
	}

	MSG_CS_Get_VerificationCodeEx* pVerificationCode = (MSG_CS_Get_VerificationCodeEx*)pData;
	
	MSG_SC_VerificationCode result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_VERIFICATION_CODE;
	result.uValue1 = pVerificationCode->uClientSock;

	//检查手机号码是否合法
	std::string strMobileFullNum(pVerificationCode->szMobileNum);
	std::string strCountryCode;
	std::string strMobileNum;
	bool bSplit = SplitPhoneNum(strMobileFullNum, strCountryCode, strMobileNum);
	if (!bSplit)
    {
        result.cbResult = 1;
        SendMsg(pSocket, &result, sizeof(result));
        return;
    }

	std::string strCheckMobileFullNum = std::string("+")+strCountryCode+strMobileNum;
	if (!IsMobileNum(strCheckMobileFullNum))
	{
		result.cbResult = 1;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}
	
	CT_BYTE cbBind = GoDBCheckMobileBind(pVerificationCode->cbPlatformId, pVerificationCode->szMobileNum);
	if (pVerificationCode->cbType == 1)//判断手机号是否已经被绑定
	{
		if (cbBind != 0)
		{
			result.cbResult = 2;
			SendMsg(pSocket, &result, sizeof(result));
			return;
		}
	}
	else if (pVerificationCode->cbType == 2 || pVerificationCode->cbType == 3)//判断手机号有没有绑定
	{
		if (cbBind == 0)
		{
			result.cbResult = 3;
			SendMsg(pSocket, &result, sizeof(result));
			return;
		}
	}
	else
	{
		result.cbResult = 1;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}
	//
	//std::default_random_engine e;
	//std::uniform_int_distribution <> u(1000, 9999);
	//int nVerifyCode = u(e);
	// 以随机值播种，若可能
	std::random_device r;
	// 选择 1000 与 9999 间的随机数
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> uniform_dist(1000, 9999);
	int nVerifyCode = uniform_dist(e1);

	CVerificationCodeThread* thr = new CVerificationCodeThread(true);
	thr->set_detachable(true);
    std::shared_ptr<tagVerificationCode> verificationCodePtr = std::make_shared<tagVerificationCode>(strCountryCode, strMobileNum, pVerificationCode->cbPlatformId, nVerifyCode);
    thr->SetVerificationCode(verificationCodePtr);
	if (thr->start() == false)
	{
		LOG(WARNING) << "start thread failed! mobile num: " << strMobileNum << ", verifycationCode: " << nVerifyCode;
	}

	// old version modify in 2019-04-19
    /*std::shared_ptr<tagVerificationCode> verificationCodePtr = m_VerificationCodeThreadPtr->GetFreeVerificationCodeQue();
    verificationCodePtr->nPlatformId = pVerificationCode->cbPlatformId;
    verificationCodePtr->strMobileNum = strMobileNum;
    verificationCodePtr->nVerificationCode = nVerifyCode;
    m_VerificationCodeThreadPtr->InsertVerificationCodeQue(verificationCodePtr);*/

	SetMobileVerifyCode(pVerificationCode->cbPlatformId, pVerificationCode->szMobileNum, nVerifyCode);
	result.cbResult = 0;
	SendMsg(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserResetPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Reset_PwdEx))
	{
		return;
	}

	MSG_CS_Reset_PwdEx* pResetPwd = (MSG_CS_Reset_PwdEx*)pData;

	MSG_SC_Reset_Pwd result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_RESET_PWD;
	result.uValue1 = pResetPwd->uClientSock;

	/*CT_BYTE cbBind = GoDBCheckMobileBind(pResetPwd->szMobileNum);
	if (cbBind == 0)
	{
		result.cbResult = 1;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}*/

	//判断帐号密码是否为空
	if (strlen(pResetPwd->szMobileNum) == 0 || strlen(pResetPwd->szPassword) == 0)
	{
		result.cbResult = 3;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}

	//检测验证码
	CT_DWORD dwVerifyCode = 0;
	CT_BOOL	bGetCode = GetMobileVerifyCode(pResetPwd->cbPlatformId, pResetPwd->szMobileNum, dwVerifyCode);
	if (!bGetCode || dwVerifyCode != pResetPwd->dwVerifyCode)
	{
		result.cbResult = 2;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}

	//获取玩家平台ID
	//CT_BYTE cbPlatformId = GetUserPlatformId(cbPlatformId)

	//修改数据库密码
	CT_BYTE cbReset = GoDBResetPwd(pResetPwd);
	if (cbReset != 0)
	{
		result.cbResult = 1;
		SendMsg(pSocket, &result, sizeof(result));
		return;
	}

	//修改缓存密码
	CT_DWORD dwUserID = 0;
	SetUserPwd(pResetPwd->szMobileNum, pResetPwd->szPassword, dwUserID, pResetPwd->cbPlatformId);

	//删除验证码
	DeleteMobileVerifyCode(pResetPwd->cbPlatformId, pResetPwd->szMobileNum);

	result.cbResult = 0;
	SendMsg(pSocket, &result, sizeof(result));

	//如果是修改，则做记录
	InsertModifyRecord(dwUserID, 9, pResetPwd->szPassword);
}

CT_VOID CLoginServerThread::OnUserModifyPersonalInfo(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Modify_PersonalInfo))
	{
		return;
	}

	MSG_CS_Modify_PersonalInfo* pModify = (MSG_CS_Modify_PersonalInfo*)pData;

	//修改个人信息
	MSG_SC_Modify_PersonalInfo result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_MODIFY_PERSONAL_INFO;
	result.dwValue2 = pModify->dwUserID;

	if (pModify->cbSex != en_Boy && pModify->cbSex != en_Girl)
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	//男女头像统一从1开始
	if (!(pModify->cbHeadId >= en_HeadId_Boy_Start && pModify->cbHeadId < en_HeadId_Boy_Start + en_HeadId_Count))
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	
	/*if (pModify->cbSex == en_Boy)
	{
		if (!(pModify->cbHeadId >= en_HeadId_Boy_Start && pModify->cbHeadId < en_HeadId_Boy_Start + en_HeadId_Count))
		{
			result.cbResult = 2;
			SendMsgByUserID(pSocket, &result, sizeof(result));
			return;
		}
	}

	if (pModify->cbSex == en_Girl)
	{
		if (!(pModify->cbHeadId >= en_HeadId_Girl_Start && pModify->cbHeadId < en_HeadId_Girl_Start + en_HeadId_Count))
		{
			result.cbResult = 2;
			SendMsgByUserID(pSocket, &result, sizeof(result));
			return;
		}
	}*/

	//修改db数据
	if (GoDBModifyPersonalInfo(pModify) != 0)
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	SetUserPersonalInfo(pModify);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
}

CT_VOID CLoginServerThread::OnUserBindAliPay(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_AliPay))
	{
		return;
	}

	MSG_CS_Bind_AliPay* pBindAliPay = (MSG_CS_Bind_AliPay*)pData;

	MSG_SC_Bind_ExchangeAccount bindAccount;
	bindAccount.dwMainID = MSG_LOGIN_MAIN;
	bindAccount.dwSubID = SUB_SC_BIND_EXCHANGE_ACCOUNT;
	bindAccount.dwValue2 = pBindAliPay->dwUserID;

	std::string strAliPay(pBindAliPay->szAliPayAccount);
	if (strAliPay.find("@") == std::string::npos)
	{
		if (!Utility::isDigits(strAliPay) || strAliPay.length() != 11)
		{
			bindAccount.cbResult = 1;
			SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
			return;
		}
	}

	if (pBindAliPay->cbType != 1 && pBindAliPay->cbType != 2)
	{
		bindAccount.cbResult = 1;
		SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
		return;
	}

	//修改
	if (pBindAliPay->cbType == 2)
	{
		acl::string strAccount;
		GetUserAliPayAccount(pBindAliPay->dwUserID, strAccount);
		if (strAccount.empty())
		{
			bindAccount.cbResult = 3;
			SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
			return;
		}

		//检测验证码
		/*acl::string strMobileNum;
		GetUserMobileNum(pBindAliPay->dwUserID, strMobileNum);
		if (strMobileNum.empty())
		{
			bindAccount.cbResult = 4;
			SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
			return;
		}

		CT_DWORD dwVerifyCode = GetMobileVerifyCode(strMobileNum.c_str());
		if (dwVerifyCode != pBindAliPay->dwVerifyCode)
		{
			bindAccount.cbResult = 4;
			SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
			return;
		}*/
	}

	//设置数据库
	CT_BYTE bBind = GoDBBindExchangeAliPay(pBindAliPay);
	if (bBind != 0)
	{
		bindAccount.cbResult = 1;
		SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
		return;
	}

	//设置缓存
	SetUserAliPay(pBindAliPay);

	bindAccount.cbResult = 0;
	SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
	

	//如果是修改，则做记录
	if (pBindAliPay->cbType == 2)
	{
		InsertModifyRecord(pBindAliPay->dwUserID, 2, pBindAliPay->szAliPayAccount);
	}
}

CT_VOID CLoginServerThread::OnUserBindBankCard(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_BankCard))
	{
		return;
	}

	MSG_CS_Bind_BankCard* pBindBankCard = (MSG_CS_Bind_BankCard*)pData;

	MSG_SC_Bind_ExchangeAccount bindAccount;
	bindAccount.dwMainID = MSG_LOGIN_MAIN;
	bindAccount.dwSubID = SUB_SC_BIND_EXCHANGE_ACCOUNT;
	bindAccount.dwValue2 = pBindBankCard->dwUserID;

	std::string strBankCard(pBindBankCard->szBankCard);
	if (!Utility::isDigits(strBankCard))
	{
		bindAccount.cbResult = 2;
		SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
		return;
	}
	//设置数据库
	CT_BYTE bBind = GoDBBindExchangeBankCard(pBindBankCard);
	if (bBind != 0)
	{
		bindAccount.cbResult = 2;
		SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));
		return;
	}

	CT_BOOL bModify = false;
	acl::string strOldBankCard;
	GetUserBankCard(pBindBankCard->dwUserID, strOldBankCard);
	if (!strOldBankCard.empty())
	{
		bModify = true;
	}

	//设置缓存
	SetUserBankCard(pBindBankCard);

	bindAccount.cbResult = 0;
	SendMsgByUserID(pSocket, &bindAccount, sizeof(bindAccount));

	//如果是修改，则做记录
	if (bModify)
	{
		InsertModifyRecord(pBindBankCard->dwUserID, 3, pBindBankCard->szBankCard);
	}
}

CT_VOID CLoginServerThread::OnUserExchangeScoreToRmb(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Exchange_Score_To_Rmb))
	{
		return;
	}

	MSG_CS_Exchange_Score_To_Rmb *pChangeRmb = (MSG_CS_Exchange_Score_To_Rmb*)pData;
	
	MSG_SC_Exchange_To_Rmb result;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_EXCHANGE_RMB_RESULT;
	result.dwValue2 = pChangeRmb->dwUserID;
	
	tagExchangeChannelAmount* pChannelAmount = NULL;
	for (auto& it : m_vecExchangeInfo)
	{
		if (it.cbChannelID == pChangeRmb->cbType)
		{
			pChannelAmount = &it;
			break;
		}
	}
	
	if (!pChannelAmount)
	{
		result.cbResult = 4;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (pChangeRmb->cbType != 2 && pChangeRmb->cbType != 3)
	{
		result.cbResult = 4;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
 
	if (pChannelAmount->cbDayCountLimite != 0 && GetTodayExchangeOrderCount(pChangeRmb->dwUserID, pChangeRmb->cbType) >= pChannelAmount->cbDayCountLimite)
    {
	    if (pChangeRmb->cbType == 2)
        {
            result.cbResult = 7;
        } else
        {
            result.cbResult = 8;
        }
        SendMsgByUserID(pSocket, &result, sizeof(result));
        return;
    }
  

	acl::string strAccount;
	if (pChangeRmb->cbType == 2)
	{
		GetUserAliPayAccount(pChangeRmb->dwUserID, strAccount);
		if (strAccount.empty())
		{
			result.cbResult = 2;
			SendMsgByUserID(pSocket, &result, sizeof(result));
			return;
		}
		//GetTodayExchangeOrderCount(pChangeRmb->dwUserID);
	}

	if (pChangeRmb->cbType == 3)
	{
		GetUserBankCard(pChangeRmb->dwUserID, strAccount);
		if (strAccount.empty())
		{
			result.cbResult = 3;
			SendMsgByUserID(pSocket, &result, sizeof(result));
			return;
		}
		
		//银联兑换必须500元起
		CT_LONGLONG llExchangeScore = pChangeRmb->dwExchangeScore*TO_LL;
		CT_LONGLONG llMinExchangeScore = pChannelAmount->dwMinAmount*TO_LL;
		if (llExchangeScore < llMinExchangeScore)
		{
			result.cbResult = 9;
			SendMsgByUserID(pSocket, &result, sizeof(result));
			return;
		}
	}

	if (CheckUserInGameOnline(pChangeRmb->dwUserID))
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	
	CT_LONGLONG llScore = GetUserScore(pChangeRmb->dwUserID);
	CT_LONGLONG llExchangeScore = pChangeRmb->dwExchangeScore*TO_LL;
	if (llExchangeScore > llScore || llScore < 5600)
	{
		result.cbResult = 4;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	if (pChangeRmb->dwExchangeScore < pChannelAmount->dwMinAmount)
	{
		result.cbResult = 9;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	//判断玩家身上剩余金币是否超过6块
	if (llScore - llExchangeScore < 600)
	{
		result.cbResult = 4;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}

	//检测支付帐号是否被封
	/*CT_BOOL bForbid = CheckAliPayOrBankCardLimited(strAccount.c_str());
	if (bForbid)
	{
		if (pChangeRmb->cbType == 2)
		{
			result.cbResult = 5;
		}
		else
		{
			result.cbResult = 6;
		}
		
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}*/

	CT_LONGLONG llBankScore = GetUserBankScore(pChangeRmb->dwUserID);
	//金币变化记录
	InsertScoreChangeRecord(pChangeRmb->dwUserID, llBankScore, llScore, 0, -llExchangeScore, EXCHANGE_SCORE);

	//改变金币
	AddUserGemAndScore(pSocket, pChangeRmb->dwUserID, 0, -llExchangeScore, true);

	//数据库插入记录
	InsertExchangeOrder(pChangeRmb);

	result.cbResult = 0;
	SendMsgByUserID(pSocket, &result, sizeof(result));
	return;
}

CT_VOID CLoginServerThread::OnUserExchangeScoreRecord(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Exchange_RMB_Record))
	{
		return;
	}

	MSG_CS_Exchange_RMB_Record* pRecord = (MSG_CS_Exchange_RMB_Record*)pData;

	std::vector<MSG_SC_Exchange_Rmb_Record> vecRecord;
	QueryExchangeOrderRecord(pRecord->dwUserID, vecRecord);

	if (!vecRecord.empty())
	{
		CT_BYTE szBuffer[2048] = { 0 };
		CT_DWORD dwSendSize = 0;

		//兑换记录
		MSG_GameMsgDownHead msgHead;
		msgHead.dwMainID = MSG_LOGIN_MAIN;
		msgHead.dwSubID = SUB_SC_EXCHANGE_RMB_RECORD;
		msgHead.dwValue2 = pRecord->dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		for (auto& it : vecRecord)
		{
			MSG_SC_Exchange_Rmb_Record record;
			record.dwExchangeScore = it.dwExchangeScore;
			record.cbState = it.cbState;
			_snprintf_info(record.szAccount, sizeof(record.szAccount), "%s", it.szAccount);
			_snprintf_info(record.szTime, sizeof(record.szTime), "%s", it.szTime);

			memcpy(szBuffer + dwSendSize, &record, sizeof(MSG_SC_Exchange_Rmb_Record));
			dwSendSize += sizeof(MSG_SC_Exchange_Rmb_Record);
		}
		SendMsgByUserID(pSocket, szBuffer, dwSendSize);
	}
}

CT_VOID CLoginServerThread::OnUserLogout(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_P2LS_UserLogout))
	{
		return;
	}
	
	MSG_P2LS_UserLogout* pUserLogout = (MSG_P2LS_UserLogout*)pData;

	CT_BOOL bSucc = DelProxyServerInfo(pUserLogout->dwUserID, pUserLogout->dwProxyServerID);
	if (bSucc)
	{
		//删除玩家的session信息
		//DelUserSession(pUserLogout->dwUserID);

		//设置玩家帐号信息的过期时间
		SetAccountInfoExpire(pUserLogout->dwUserID);

		/*
		if (m_cbScoreMode)
		{
		//设置救济金信息过期时间
		SetBenefitExpire(pUserLogout->dwUserID);

		//设置玩家金币信息过期时间
		SetScoreInfoExpire(pUserLogout->dwUserID);

		//设置任务过期时间
		SetTaskExpire(pUserLogout->dwUserID);
		}
		*/
		//设置任务过期时间
		SetTaskExpire(pUserLogout->dwUserID);

		//设置賺金过期时间
		SetEarnScoreExpire(pUserLogout->dwUserID);

		//设置玩家大局信息过期时间
		//SetGameRoundCountExpire(pUserLogout->dwUserID);
	}
}

CT_VOID CLoginServerThread::OnUserUnRedExchange(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_UnRed_Exchange))
	{
		return;
	}

	MSG_CS_UnRed_Exchange* pUnRedExchange = (MSG_CS_UnRed_Exchange*)pData;

	//修改缓存数据
	acl::string key;
	key.format("account_%u", pUnRedExchange->dwUserID);

	m_redis.clear();
	acl::string strShowExchange;
	strShowExchange.format("%d", 2);
	if (m_redis.hset(key.c_str(), "exchange", strShowExchange.c_str()) == -1)
	{
		LOG(WARNING) << "update user show exchange fail, user id: " << pUnRedExchange->dwUserID << ", show: " << 2;
	}

	//修改数据库数据

	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}
	do
	{
		acl::query query;
		query.create_sql("update userinfo set showExchange = :showExchange where userid=:userid limit 1")
		.set_format("showExchange", "%d", 2)
		.set_format("userid", "%u", pUnRedExchange->dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user show exchange to db fail, show: " << 2 << ", user id: " << pUnRedExchange->dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
		}
	} while (0);

	pool->put(db);

	//给玩家返回数据
	MSG_SC_ShowExchange showExchange;
	showExchange.dwMainID = MSG_FRAME_MAIN;
	showExchange.dwSubID = SUB_S2C_SHOW_EXCHANGE;
	showExchange.dwValue2 = pUnRedExchange->dwUserID;
	showExchange.cbShowExchange = 2;
	SendMsgByUserID(pSocket, &showExchange, sizeof(showExchange));
}

CT_VOID CLoginServerThread::OnUserReportUser(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
    if (wDataSize != sizeof(MSG_CS_Report_User_ForPlaza))
    {
        return;
    }
    
    MSG_CS_Report_User_ForPlaza* pReportUser = (MSG_CS_Report_User_ForPlaza*)pData;
	
	MSG_SC_Report_User_ForPlaza result;
	result.dwDrawID = pReportUser->dwDrawID;
	result.wGameID = pReportUser->wGameID;
	result.wKindID = pReportUser->wKindID;
	result.dwMainID = MSG_LOGIN_MAIN;
	result.dwSubID = SUB_SC_REPORT_USER_FORPLAZA;
	result.dwValue2 = pReportUser->dwUserID;
	
    //先判断玩家是否有足够的金币
    CT_LONGLONG llUserScore = GetUserScore(pReportUser->dwUserID);
	CT_LONGLONG llNeedScore = 0;
    if (pReportUser->wGameID == GAME_ZJH)
	{
		llNeedScore = 1000;
	
	}
	else
	{
		llNeedScore = 200;
	}
	
	if (llUserScore < llNeedScore)
	{
		result.cbResult = 2;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
	
	//判断玩家是否在游戏中
	if (CheckUserInGameOnline(pReportUser->dwUserID))
	{
		result.cbResult = 3;
		SendMsgByUserID(pSocket, &result, sizeof(result));
		return;
	}
    
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }
    
    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }
    
    CT_BOOL bInsert = false;
    
    do
    {
        //举报的数据是否需要太过精确
       /* CT_DWORD dwDrawID = 0;
        std::string date(Utility::GetTimeNowString());
        std::string year = date.substr(0, 4);
        std::string month = date.substr(5, 2);
        std::stringstream drawScoreTable;
        drawScoreTable << "SELECT drawid from record_score.record_draw_score_" << year << month << " where userid = " << pReportUser->dwUserID << " ORDER BY drawid desc LIMIT 1";
        
        if (db->sql_select(drawScoreTable.str().c_str()) ==false)
        {
            LOG(WARNING) << "select user last drawid fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }
    
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            dwDrawID = atoi((*row)["drawid"]);
        }
        db->free_result();
        
        if (dwDrawID == 0)
		{
		    LOG(WARNING) << "select drawid fail, sql: "  << drawScoreTable.str().c_str();
			break;
		}*/
    
        acl::query query;
       /* query.create_sql("SELECT id FROM treasure.illegal_player WHERE drawid = :drawid AND userid = :userid AND drawdate = :drawdate  LIMIT 1")
		.set_format("drawid", "%d", pReportUser->dwDrawID)
		.set_format("userid", "%u", pReportUser->dwUserID)
		.set_format("drawdate", "%s", pReportUser->szDate);*/
	
		std::string date(pReportUser->szDate);
		std::string year = date.substr(0, 4);
		std::string month = date.substr(5, 2);
		std::string drawInfoTable = ("record_score.record_draw_info_") + year + month;
		std::string drawScoreTable = ("record_score.record_draw_score_") + year + month;
		std::stringstream sql;
		sql << "SELECT " << drawInfoTable << ".gameid," <<  drawInfoTable << ".kindid," << drawInfoTable << ".roomKindid," << drawScoreTable << ".score," << drawScoreTable << ".report FROM "
		<< drawScoreTable << " INNER JOIN " << drawInfoTable << " ON " << drawInfoTable << ".drawid = " << drawScoreTable << ".drawid WHERE " << drawScoreTable << ".drawid = "
		<< pReportUser->dwDrawID << " AND " << drawScoreTable <<  ".userid = " << pReportUser->dwUserID << " LIMIT 1;";
	
		//sql << "SELECT score, report FROM record_score.record_draw_score_" << year << month << " WHERE drawid = " << pReportUser->dwDrawID << " AND userid = " << pReportUser->dwUserID
		//<< " LIMIT 1;";
		
		if (db->sql_select(sql.str().c_str()) ==false)
		{
			LOG(WARNING) << "select user drawid info fail. errno: " << db->get_errno() << ", err msg: "
						 << db->get_error();
			LOG(WARNING) << sql.str().c_str();
			break;
		}
		
 		CT_LONGLONG llUserScore = 0;
		CT_WORD wGameID = 0;
		CT_WORD wKindID = 0;
		CT_WORD wRoomKindID = 0;
		CT_BYTE cbReport = 0;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			wGameID = (CT_WORD)atoi((*row)["gameid"]);
			wKindID = (CT_WORD)atoi((*row)["kindid"]);
			wRoomKindID = (CT_WORD)atoi((*row)["roomKindid"]);
			llUserScore = atoll((*row)["score"]);
			cbReport = (CT_BYTE)atoi((*row)["report"]);
		}
		
		if (cbReport != 0)
			break;
       
		query.reset();
        query.create_sql("INSERT INTO treasure.illegal_player (drawid, drawdate, gameid, kindid, roomKindid, userid, fee, score,  fixdate) \
         VALUES (:drawid, :drawdate, :gameid, :kindid, :roomkind, :userid, :fee, :score, :fixdate)")
          .set_format("drawid", "%d", pReportUser->dwDrawID)
		  .set_format("drawdate", "%s", pReportUser->szDate)
		  .set_format("gameid", "%d", wGameID)
		  .set_format("kindid", "%d", wKindID)
		  .set_format("roomkind", "%d", wRoomKindID)
		  .set_format("fee", "%lld", llNeedScore)
		  .set_format("score", "%lld", llUserScore)
          .set_format("userid", "%u", pReportUser->dwUserID)
          .set_format("fixdate", "%u", (CT_DWORD)time(NULL));
        
        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "insert user report info fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }
	
		if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
		{
			int nError = db->get_errno();
			LOG(WARNING) << "insert report info fail, errno = " << nError << ", error: " << db->get_error();
			break;
		}
		CT_DWORD dwReportID = 0;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwReportID = atoi((*row)["LAST_INSERT_ID()"]);
		}
	
		acl::string strBeReportUser;
		strBeReportUser.format("%u", pReportUser->dwBeReportUserID[0]);
		for (auto i = 0; i < 4; ++i)
		{
			if (pReportUser->dwBeReportUserID[i] == 0)
				break;
			
			query.reset();
			query.create_sql("INSERT INTO treasure.illegal_player_list (id, player) VALUES (:reportid, :player)")
			.set_format("reportid", "%d", dwReportID)
			.set_format("player", "%u", pReportUser->dwBeReportUserID[i]);
			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "insert user report player info fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}
		}
	
		/*query.reset();
		query.create_sql("UPDATE record_score.record_draw_score_201807 SET report = 1 WHERE drawid = :drawid AND userid = :userid LIMIT 1 ")
		.set_format("drawid", "%d", pReportUser->dwDrawID)
		.set_format("userid", "%u", pReportUser->dwUserID);*/
		std::stringstream sql2;
		sql2 << "UPDATE " << drawScoreTable << " SET report = 1 WHERE drawid = " << pReportUser->dwDrawID << " AND userid = " <<  pReportUser->dwUserID << " LIMIT 1";

		if (db->sql_update(sql2.str().c_str()) == false)
		{
			LOG(WARNING) << "insert user report flag fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			LOG(WARNING) << sql2.str().c_str();
			break;
		}
        bInsert = true;
        
    } while (0);
	
    db->free_result();
	pool->put(db);
	
	if (bInsert)
	{
		//金币变化记录
		InsertScoreChangeRecord(pReportUser->dwUserID, 0, llUserScore, 0, -llNeedScore, REPORT_PLAYER);
		
		//改变金币
		AddUserGemAndScore(pSocket, pReportUser->dwUserID, 0, -llNeedScore, true);
		
		result.cbResult = 0;
		SendMsgByUserID(pSocket, &result, sizeof(result));
	}
	else
	{
		result.cbResult = 1;
		SendMsgByUserID(pSocket, &result, sizeof(result));
	}
}

CT_VOID CLoginServerThread::OnUserQueryRechargeRecord(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Query_RechargeRecord))
	{
		return;
	}

	MSG_CS_Query_RechargeRecord* pRecord = (MSG_CS_Query_RechargeRecord*)pData;

	std::vector<MSG_SC_RechargeRecord> vecRecord;
	QuyerRechargeOrderRecord(pRecord->dwUserID, vecRecord);

    CT_BYTE szBuffer[2048] = { 0 };
    CT_DWORD dwSendSize = 0;

    //充值记录
    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_LOGIN_MAIN;
    msgHead.dwSubID = SUB_SC_RECHARGE_RECORD;
    msgHead.dwValue2 = pRecord->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);
	if (!vecRecord.empty())
	{
		for (auto& it : vecRecord)
		{
			memcpy(szBuffer + dwSendSize, &it, sizeof(MSG_SC_RechargeRecord));
			dwSendSize += sizeof(MSG_SC_RechargeRecord);
		}
	}
	SendMsgByUserID(pSocket, szBuffer, dwSendSize);
}

CT_VOID CLoginServerThread::OnUpdateShowExchangeCond(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagChannelShowExchangeCond))
	{
		return;
	}

	tagChannelShowExchangeCond* pShowExchange = (tagChannelShowExchangeCond*)pData;
	m_mapChannelShowExchange[pShowExchange->dwChannelID] = *pShowExchange;
	LOG(WARNING) << "set channel show: " << pShowExchange->dwChannelID << ", cond: " << pShowExchange->dwTotalRecharge;
}

CT_VOID CLoginServerThread::OnUpdateChannelPresentScore(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagChannelPresentScore))
	{
		return;
	}

	tagChannelPresentScore* pPresentScore = (tagChannelPresentScore*)pData;
	if (pPresentScore->dwChannelID == 0)
	{
		m_iRegisterPresentScore = pPresentScore->dwRegisterScore;
		m_iBindMobileScore = pPresentScore->dwBindMobileScore;
	}
	else
	{
		stChannelPresentScore presentScore;
		presentScore.dwRegisterPresentScore = pPresentScore->dwRegisterScore;
		presentScore.dwBindMobilePresentScore = pPresentScore->dwBindMobileScore;
		m_mapChannelPresentScore[pPresentScore->dwChannelID] = presentScore;
	}

	LOG(WARNING) << "set channel present score, channel id: " << pPresentScore->dwChannelID << ", register score: "\
 	<< pPresentScore->dwRegisterScore << ", bind mobile score: " << pPresentScore->dwBindMobileScore;
}

CT_VOID CLoginServerThread::OnUpdateExchangeInfo(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagExchangeChannelAmount))
	{
		return;
	}
	
	tagExchangeChannelAmount* pExchangeInfo = (tagExchangeChannelAmount*)pData;
	for (auto it = m_vecExchangeInfo.begin(); it != m_vecExchangeInfo.end(); ++it)
	{
		if (it->cbChannelID == pExchangeInfo->cbChannelID)
		{
			m_vecExchangeInfo.erase(it);
			m_vecExchangeInfo.push_back(*pExchangeInfo);
			LOG(WARNING) << "upate exchange amount info, channel id: " << (int)pExchangeInfo->cbChannelID;
			return;
		}
	}
}

CT_VOID CLoginServerThread::OnUpdateBenefitConfig(const CT_VOID* pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagBenefitReward))
    {
        return;
    }

    tagBenefitReward* pBenefitReward = (tagBenefitReward*)pData;
    m_mapBenefitConfig[pBenefitReward->cbVip2] = *pBenefitReward;
}

CT_VOID CLoginServerThread::OnUpdateSMSPid(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CT_DWORD))
	{
		return;
	}

	CT_DWORD dwSMSPid = *(CT_DWORD*)pData;
	CVerificationCodeThread::m_dwUseIndex = dwSMSPid;
}

CT_VOID CLoginServerThread::GoCenterAddGroupUser(UserAccountsBaseData& accountData, CT_DWORD dwParentUserID)
{
    if (pNetCenter != NULL && pNetCenter->IsRunning())
    {
        MSG_L2CS_AddGroupUser addGroupUser;
        addGroupUser.dwUserID = accountData.dwUserID;
        addGroupUser.dwParentUserID = dwParentUserID;
        addGroupUser.cbSex = accountData.cbGender;
        addGroupUser.cbHeadId = accountData.cbHeadId;
        addGroupUser.cbVip2 = accountData.cbVipLevel2;
        CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_LCS_MAIN, SUB_L2CS_ADD_GROUP_USER, &addGroupUser, sizeof(MSG_L2CS_AddGroupUser));
    }
}

CT_VOID CLoginServerThread::SendMsg(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen)
{
	if (pSocket)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, pBuf, dwLen);
	}
}

CT_VOID CLoginServerThread::SendMsgByUserID(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen)
{
	if (pSocket)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, pBuf, dwLen);
	}
}

CT_VOID CLoginServerThread::SendLoginFail(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_INT32 iErrorCode)
{
	if (pSocket)
	{
		MSG_SC_LoginError loginErr;
		loginErr.dwMainID = MSG_LOGIN_MAIN;
		loginErr.dwSubID = SUB_SC_LOGIN_FAILED;
		loginErr.uValue1 = dwClientValue;
		loginErr.lErrorCode = iErrorCode;
		SendMsg(pSocket, &loginErr, sizeof(loginErr));
	}
}

std::string CLoginServerThread::toHexString(const unsigned char* input, const int datasize)
{
	std::string output;
	char ch[3];

	for (int i = 0; i < datasize; ++i)
	{
		_snprintf_info(ch, 3, "%02x", input[i]);
		output += ch;
	}
	return output;

}

CT_VOID CLoginServerThread::SendLoginSuccess(acl::aio_socket_stream* pSocket, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData)
{
	if (pSocket)
	{
		MSG_SC_LoginSuccess loginSucc;
		loginSucc.dwUserID = accountData.dwUserID;
		loginSucc.cbNewAccount = accountData.cbNewAccount;
		loginSucc.cbShowExchange = accountData.cbShowExchange;
		loginSucc.cbGender = accountData.cbGender;
		loginSucc.cbHeadId = accountData.cbHeadId;
		loginSucc.cbVipLevel = accountData.cbVipLevel;
		loginSucc.cbVipLevel2 = accountData.cbVipLevel2;
		GetNextVip2LevelInfo(loginSucc.cbVipLevel2, loginSucc.cbNextVip2, loginSucc.cbNextVip2NeedRecharge);
		loginSucc.dwRecharge = accountData.dwTotalRecharge;
		loginSucc.dScore = accountData.llScore*TO_DOUBLE;
		loginSucc.dBankScore = accountData.llBankScore*TO_DOUBLE;
		loginSucc.cbPlatformId = accountData.cbPlatformID;
		_snprintf_info(loginSucc.szNickName, sizeof(loginSucc.szNickName), "%s", accountData.szNickName);
		//_snprintf_info(loginSucc.szUUID, sizeof(loginSucc.szUUID), "%s", accountData.szUUID);

		//银行卡和支付宝信息
		_snprintf_info(loginSucc.szAliPayAccout, sizeof(loginSucc.szAliPayAccout), "%s", accountData.szAliPayAccout);
		_snprintf_info(loginSucc.szAliPayName, sizeof(loginSucc.szAliPayName), "%s", accountData.szAliPayName);
		_snprintf_info(loginSucc.szBankCardNum, sizeof(loginSucc.szBankCardNum), "%s", accountData.szBankCardNum);
		_snprintf_info(loginSucc.szBankCardName, sizeof(loginSucc.szBankCardName), "%s", accountData.szBankCardName);
		
		//判断是否开通银行密码
		if (strlen(accountData.szBankPassword) != 0)
		{
			loginSucc.cbOpenBank = 1;
		}
		else
		{
			loginSucc.cbOpenBank = 0;
		}

		//判断是否绑定手机
		if (strlen(accountData.szMobileNum) != 0)
		{
			loginSucc.cbBindMobile = 1;
		}
		else
		{
			loginSucc.cbBindMobile = 0;

			//没有绑定手机的玩家发送绑定金额
			MSG_SC_BindMobile_PresentScore presentScore;
			presentScore.dwMainID = MSG_LOGIN_MAIN;
			presentScore.dwSubID = SUB_SC_BIND_MOBILE_SCORE;
			presentScore.uValue1 = pLoginData->uClientSock;
			presentScore.dBindMobileScore = GetChannelPresentScore(accountData.dwChannelID, en_BindMobile)*TO_DOUBLE;
			SendMsg(pSocket, &presentScore, sizeof(presentScore));
		}
		
		static CT_CHAR szWebPwd[WEB_PWD_LEN] = { 0 };

		std::string strUnionID(pLoginData->szAccount);
		strUnionID.erase(0, strUnionID.size() / 2);
		_snprintf_info(szWebPwd, WEB_PWD_LEN, "{\"userid\":%u,\"unionid\":\"%s\"}", accountData.dwUserID, strUnionID.c_str());

		static CT_CHAR szBuffer[WEB_PWD_LEN * 2] = { 0 };
		memset(szBuffer, 0, sizeof(szBuffer));

		m_encrypt.encrypt((CT_UCHAR*)szWebPwd, (CT_INT32)strlen(szWebPwd), (CT_UCHAR*)szBuffer);
		std::string encryptStr = toHexString((CT_UCHAR*)szBuffer, (int)strlen(szWebPwd));
		_snprintf_info(loginSucc.szWebPwd, sizeof(loginSucc.szWebPwd)-1, "%s", encryptStr.c_str());
		
		loginSucc.dwMainID = MSG_LOGIN_MAIN;
		loginSucc.dwSubID = SUB_SC_LOGIN_SUCCESS;
		loginSucc.uValue1 = pLoginData->uClientSock;
		loginSucc.dwValue2 = Utility::ip2Int(pLoginData->szClientIP);
		SendMsg(pSocket, &loginSucc, sizeof(loginSucc));
	}
}

CT_VOID CLoginServerThread::SendGameItem(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID)
{
	if (pSocket)
	{
		std::vector<MSG_SC_GameItem> vecGameItem;
		CServerMgr::get_instance().EmumGameKindItem(dwGameID, vecGameItem);

		CT_WORD wSendSize = 0;
		CT_BYTE cbDataBuffer[SOCKET_TCP_PACKET];
		MSG_GameMsgDownHead MsgHead;
		MsgHead.dwMainID = MSG_LOGIN_MAIN;
		MsgHead.dwSubID = SUB_SC_GAME_KIND;
		MsgHead.uValue1 = dwClientValue;
		memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
		wSendSize += sizeof(MsgHead);
		for (auto it :  vecGameItem)
		{
			if (wSendSize + sizeof(MSG_SC_GameItem) > sizeof(cbDataBuffer))
			{
				SendMsg(pSocket, cbDataBuffer, wSendSize);
				wSendSize = 0;
				memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
				wSendSize += sizeof(MsgHead);
			}
			MSG_SC_GameItem& gameItem = it;
			memcpy(cbDataBuffer + wSendSize, &gameItem, sizeof(gameItem));
			wSendSize += sizeof(gameItem);
		}

		if (wSendSize > 0)
		{
			SendMsg(pSocket, cbDataBuffer, wSendSize);
		}
	}
}

CT_VOID CLoginServerThread::SendGameRoomKind(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID)
{
	if (pSocket)
	{
		std::vector<MSG_SC_RoomKindItem> vecGameRoomItem;
		CServerMgr::get_instance().EmumGameRoomKindItem(dwGameID, vecGameRoomItem);

		CT_WORD wSendSize = 0;
		CT_BYTE cbDataBuffer[SOCKET_TCP_PACKET];
		MSG_GameMsgDownHead MsgHead;
		MsgHead.dwMainID = MSG_LOGIN_MAIN;
		MsgHead.dwSubID = SUB_SC_ROOM_KIND;
		MsgHead.uValue1 = dwClientValue;
		memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
		wSendSize += sizeof(MsgHead);
		for (auto it : vecGameRoomItem)
		{
			if (wSendSize + sizeof(MSG_SC_RoomKindItem) > sizeof(cbDataBuffer))
			{
				SendMsg(pSocket, cbDataBuffer, wSendSize);
				wSendSize = 0;
				memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
				wSendSize += sizeof(MsgHead);
			}
			MSG_SC_RoomKindItem& roomItem = it;
			memcpy(cbDataBuffer + wSendSize, &roomItem, sizeof(roomItem));
			wSendSize += sizeof(roomItem);
		}

		if (wSendSize > 0)
		{
			SendMsg(pSocket, cbDataBuffer, wSendSize);
		}
	}
}

CT_VOID CLoginServerThread::SendGamePrivateRoomInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID)
{
	if (pSocket)
	{
		std::vector<MSG_SC_PrivateRoomInfo> vecPrivateRoomItem;
		CServerMgr::get_instance().EmumPrivateRoomInfo(dwGameID, vecPrivateRoomItem);

		CT_WORD wSendSize = 0;
		CT_BYTE cbDataBuffer[SOCKET_TCP_PACKET];
		MSG_GameMsgDownHead MsgHead;
		MsgHead.dwMainID = MSG_LOGIN_MAIN;
		MsgHead.dwSubID = SUB_SC_PRIVATE_ROOM_INFO;
		MsgHead.uValue1 = dwClientValue;
		memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
		wSendSize += sizeof(MsgHead);
		for (auto it : vecPrivateRoomItem)
		{
			if (wSendSize + sizeof(MSG_SC_PrivateRoomInfo) > sizeof(cbDataBuffer))
			{
				SendMsg(pSocket, cbDataBuffer, wSendSize);
				wSendSize = 0;
				memcpy(cbDataBuffer, &MsgHead, sizeof(MSG_GameMsgDownHead));
				wSendSize += sizeof(MsgHead);
			}
			MSG_SC_PrivateRoomInfo& privateRoom = it;
			memcpy(cbDataBuffer + wSendSize, &privateRoom, sizeof(privateRoom));
			wSendSize += sizeof(privateRoom);
		}

		if (wSendSize > 0)
		{
			SendMsg(pSocket, cbDataBuffer, wSendSize);
		}
	}
}


CT_VOID CLoginServerThread::SendSelfPrivateRoomInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID)
{
	acl::string key;
	acl::string val;
	key.format("proom_%u", dwUserID);

	m_redis.clear();

	if (m_redis.exists(key) == false)
	{
		return;
	}
	
	if (m_redis.get(key, val) == false)
	{
		return;
	}

	key.format("proom_info_%s", val.c_str());
	std::map<acl::string, acl::string> result;
	m_redis.clear();
	if (m_redis.hgetall(key, result) == false)
	{
		return;
	}

	MSG_SC_SelfPrivateRoomInfo selfRoomInfo;
	selfRoomInfo.dwMainID = MSG_LOGIN_MAIN;
	selfRoomInfo.dwSubID = SUB_SC_SELF_PROOM_INFO;
	selfRoomInfo.uValue1 = dwClientValue;
	selfRoomInfo.dwRoomNum = atoi(val.c_str());
	selfRoomInfo.wGameID = atoi(result["gameid"].c_str());
	selfRoomInfo.wKindID = atoi(result["kindid"].c_str());

	SendMsg(pSocket, &selfRoomInfo, sizeof(MSG_SC_SelfPrivateRoomInfo));
}

CT_VOID CLoginServerThread::SendUserOnlineInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, std::map<acl::string, acl::string>& mapOnlineInfo)
{
	if (mapOnlineInfo.empty())
	{
		return;
	}

	CT_WORD wGameID = atoi(mapOnlineInfo["gameid"].c_str());
	CT_WORD wKindID = atoi(mapOnlineInfo["kindid"].c_str());
	CT_WORD wRoomKindID = atoi(mapOnlineInfo["roomkind"].c_str());
	CT_DWORD dwRoomNumOrServerID = atoi(mapOnlineInfo["roomnum"].c_str());

	if (wRoomKindID == PRIVATE_ROOM)
	{
		MSG_SC_SelfPrivateRoomInfo selfRoomInfo;
		selfRoomInfo.dwMainID = MSG_LOGIN_MAIN;
		selfRoomInfo.dwSubID = SUB_SC_SELF_PROOM_INFO;
		selfRoomInfo.uValue1 = dwClientValue;
		selfRoomInfo.dwRoomNum = dwRoomNumOrServerID;
		selfRoomInfo.wGameID = wGameID;
		selfRoomInfo.wKindID = wKindID;
		SendMsg(pSocket, &selfRoomInfo, sizeof(MSG_SC_SelfPrivateRoomInfo));
	}

	MSG_SC_OnlineInfo onlineInfo;
	onlineInfo.dwMainID = MSG_LOGIN_MAIN;
	onlineInfo.dwSubID = SUB_SC_ONLINE_INFO;
	onlineInfo.uValue1 = dwClientValue;
	onlineInfo.wGameID = wGameID;
	onlineInfo.wKindID = wKindID;
	onlineInfo.wRoomKindID = wRoomKindID;
	onlineInfo.dwRoomNumOrServerID = dwRoomNumOrServerID;

	SendMsg(pSocket, &onlineInfo, sizeof(MSG_SC_OnlineInfo));
}

CT_VOID CLoginServerThread::SendUserSignInInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID)
{
	if (m_signInData.bNewDate == true)
	{
		/*MSG_SC_SignIn_Info signIn;
		signIn.dwMainID = MSG_LOGIN_MAIN;
		signIn.dwSubID = SUB_SC_SIGN_IN_INFO;
		signIn.uValue1 = dwClientValue;
		signIn.cbSignInDays = m_signInData.cbSignInDays;
		signIn.wRewardGem = m_signInData.wRewardGem;
		signIn.dwRewardScore = m_signInData.dwRewardScore;
		SendMsg(pSocket, &signIn, sizeof(signIn));
		m_signInData.Reset();*/
	}
}

CT_VOID CLoginServerThread::SendUserWelfareInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID)
{
	MSG_SC_Welfare_Info welfareInfo;
	welfareInfo.dwMainID = MSG_LOGIN_MAIN;
	welfareInfo.dwSubID = SUB_SC_WELFARE_INFO;
	welfareInfo.uValue1 = dwClientValue;

	welfareInfo.wSignInDays = m_signInData.wSignInDays;
	welfareInfo.cbTodaySignIn = (!m_signInData.bNewDate);

	//welfareInfo.cbBenefitCount = m_benefitData.cbCurrCount;
	//welfareInfo.cbBenefitTotalCount = m_benefitCofig.cbRewardCount;
	//welfareInfo.dwRewardScore = m_benefitCofig.dwRewardScore;
	//welfareInfo.dwLessScore = m_benefitCofig.dwLessScore;

	SendMsg(pSocket, &welfareInfo, sizeof(welfareInfo));

	m_signInData.Reset(); 
	m_benefitData.Reset();
}

CT_VOID CLoginServerThread::SendUserScoreInfo(acl::aio_socket_stream* pSocket, UserAccountsBaseData& accountData, CT_UINT64 dwClientValue)
{
	MSG_SC_UserBaseScore baseScoreInfo;
	baseScoreInfo.dwMainID = MSG_LOGIN_MAIN;
	baseScoreInfo.dwSubID = SUB_SC_BASE_SCORE_INFO;
	baseScoreInfo.uValue1 = dwClientValue;

	baseScoreInfo.dwWinCount = m_scoreData.dwWinCount;
	baseScoreInfo.dwLostCount = m_scoreData.dwLostCount;
	baseScoreInfo.dwDrawCount = m_scoreData.dwDrawCount;
	baseScoreInfo.dwFleeCount = m_scoreData.dwFleeCount;
	baseScoreInfo.cbVipLevel = accountData.cbVipLevel;
	baseScoreInfo.dwTotalRecharge = accountData.dwTotalRecharge;

	SendMsg(pSocket, &baseScoreInfo, sizeof(baseScoreInfo));
}

CT_VOID CLoginServerThread::SendUserMail(acl::aio_socket_stream* pSocket, CT_UINT64 dwClientValue, CT_DWORD dwUserID)
{	
	if (!m_vecUserMail.empty())
	{
		//static CT_BYTE cbBuffer[]
		static CT_BYTE cbBuffer[SOCKET_TCP_PACKET];
		CT_DWORD dwSendSize = 0;

		MSG_GameMsgDownHead mailHeader;
		mailHeader.dwMainID = MSG_LOGIN_MAIN;
		mailHeader.dwSubID = SUB_SC_MAIL_INFO;
		mailHeader.uValue1 = dwClientValue;
		memcpy(cbBuffer + dwSendSize, &mailHeader, sizeof(mailHeader));
		dwSendSize += sizeof(MSG_GameMsgDownHead);
		for (auto& it : m_vecUserMail)
		{
			//it.dwMainID = MSG_LOGIN_MAIN;
			//it.dwSubID = SUB_SC_MAIL_INFO;
			//it.uValue1 = dwClientValue;
			memcpy(cbBuffer + dwSendSize, &it, sizeof(MSG_SC_Mail_Info));
			dwSendSize += sizeof(MSG_SC_Mail_Info);
			
		}
		SendMsg(pSocket, cbBuffer, dwSendSize);
		m_vecUserMail.clear();
	}
	else
	{
		MSG_GameMsgDownHead mailFinish;
		mailFinish.dwMainID = MSG_LOGIN_MAIN;
		mailFinish.dwSubID = SUB_SC_MAIL_INFO_FINISH;
		mailFinish.uValue1 = dwClientValue;
		SendMsg(pSocket, &mailFinish, sizeof(mailFinish));
	}
}

CT_VOID CLoginServerThread::SendUserGameVersion(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue)
{
	CT_BYTE szBuffer[512] = { 0 };
	CT_DWORD dwSendSize = 0;

	//红包信息
	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_LOGIN_MAIN;
	msgHead.dwSubID = SUB_SC_GAME_VERSION;
	msgHead.uValue1 = uClientValue;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	for (auto& it : m_mapGameVersion)
	{
		MSG_SC_GameVersion gameVersion;
		gameVersion.wGameID = it.first;
		gameVersion.wVersion = it.second;

		memcpy(szBuffer + dwSendSize, &gameVersion, sizeof(MSG_SC_GameVersion));
		dwSendSize += sizeof(MSG_SC_GameVersion);
	}
	SendMsg(pSocket, szBuffer, dwSendSize);
}

CT_VOID CLoginServerThread::SendUserBenefit(acl::aio_socket_stream* pSocket, UserAccountsBaseData& accountData, CT_UINT64 uClinetValue)
{
    tagBenefitReward* pBenefitReward = GetBenefitReward(m_benefitData.cbVip2);
    if (pBenefitReward != NULL)
    {
        //判断玩家分数是否够领取
        if ((accountData.llBankScore + accountData.llScore) >= pBenefitReward->dwLessScore)
        {
            return;
        }

        CT_DWORD dwNow = time(NULL);
        bool bAcrossDay = isAcrossTheDay(m_benefitData.dwLastTime, dwNow);
        if (bAcrossDay)
        {
            m_benefitData.cbCurrCount = 0;
        }

        if (m_benefitData.cbCurrCount >= pBenefitReward->cbRewardCount)
        {
            return;
        }

        MSG_SC_HasBenefit hasBenefit;
        hasBenefit.dwMainID = MSG_FRAME_MAIN;
        hasBenefit.dwSubID = SUB_S2C_HAS_BENEFIT;
        hasBenefit.cbState = 1;
        hasBenefit.uValue1 = uClinetValue;
        SendMsg(pSocket, &hasBenefit, sizeof(MSG_SC_HasBenefit));
    }
}

CT_VOID CLoginServerThread::SendUserAllSucMsg(acl::aio_socket_stream* pSocket, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData, std::map<acl::string, acl::string>& mapOnlineInfo)
{
	if (pLoginData)
	{
		SendUserOnlineInfo(pSocket,pLoginData->uClientSock, mapOnlineInfo);
		SendProxyServerLoginSucc(pSocket, pLoginData->uClientSock, accountData, 0);
		SendUserGameVersion(pSocket, pLoginData->uClientSock);
		SendLoginSuccess(pSocket, pLoginData, accountData);

		/*	
		if (m_cbScoreMode)
		{
			QuerySignInInfo(accountData.dwUserID);
			GetUserBenefitInfo(accountData.dwUserID);
			SendUserWelfareInfo(pSocket, pLoginData->uClientSock, accountData.dwUserID);
			SendUserScoreInfo(pSocket, accountData, pLoginData->uClientSock);
		}
		*/

		QueryUserMail(accountData.dwUserID, accountData.dwChannelID, m_vecUserMail);
		SendUserMail(pSocket, pLoginData->uClientSock, accountData.dwUserID);
        SendUserBenefit(pSocket, accountData, pLoginData->uClientSock);

		//插入登录日志
		InsertLoginRecord(accountData.dwUserID, pLoginData);
	}	
}

CT_VOID CLoginServerThread::AddUserGemAndScore(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore, CT_BOOL bUpdateToClient)
{
	acl::db_pool* accountPool = GetAccountdbPool();
	if (accountPool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* accountdb = accountPool->peek_open();
	if (accountdb == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}
	AddUserGemAndScoreTodb(accountdb,dwUserID, nAddGem, llAddScore);
	accountPool->put(accountdb);

	// 更新redis的玩家钻石信息
	AddUserGemAndScoreToCache(pSocket, dwUserID, nAddGem, llAddScore,bUpdateToClient);

	//更新中心服玩家数据
	AddUserGemAndScoreToCenterServer(dwUserID, nAddGem, llAddScore);
}

CT_VOID CLoginServerThread::AddUserBankScore(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llAddBankScore)
{
	//更新数据库银行金币
	AddUserBankScoreTodb(dwUserID, llAddBankScore);

	//更新缓存金币
	AddUserBankScoreToCache(pSocket, dwUserID, llAddBankScore);

	//更新中心服玩家数据
	AddBankScoreToCenterServer(dwUserID, llAddBankScore);
}

CT_VOID CLoginServerThread::UpdateUserGemToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_DWORD dwNewGem)
{
	if (pSocket)
	{
		MSG_SC_GameUserGem updateGem;
		updateGem.dwMainID = MSG_FRAME_MAIN;
		updateGem.dwSubID = SUB_S2C_USER_GEM;
		updateGem.dwUserID = dwUserID;
		updateGem.dwGem = dwNewGem;
		updateGem.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateGem, sizeof(updateGem));
	}
}

CT_VOID CLoginServerThread::UpdateUserScoreToClient(acl::aio_socket_stream* pSocket,  CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_LONGLONG llNewScore)
{
	MSG_SC_UserScoreInfo updateScore;
	updateScore.dwMainID = MSG_FRAME_MAIN;
	updateScore.dwSubID = SUB_S2C_USER_SCORE;
	updateScore.dwUserID = dwUserID;
	updateScore.wTableID = INVALID_CHAIR;
	updateScore.wChairID = INVALID_CHAIR;
	updateScore.dScore = llNewScore*0.01f;
	updateScore.dwValue2 = dwUserID;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateScore, sizeof(updateScore));
}

CT_VOID CLoginServerThread::UpdateUserBankScoreToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llNewBankScore)
{
	MSG_SC_Update_BankScore bankScore;
	bankScore.dwMainID = MSG_LOGIN_MAIN;
	bankScore.dwSubID = SUB_SC_UPDATE_BANK_SCORE;
	bankScore.dBankScore = llNewBankScore*0.01f;
	bankScore.dwValue2 = dwUserID;
	SendMsgByUserID(pSocket, &bankScore, sizeof(bankScore));
}

CT_VOID CLoginServerThread::SetUserSession(CT_DWORD dwUserID, const std::string& strUUID)
{
	std::stringstream strKey;
	strKey << "session_" << dwUserID;
	m_redis.clear();
	if (m_redis.set(strKey.str().c_str(), strUUID.c_str()) == false)
	{
		LOG(WARNING) << "set uuid failed, userid: " << dwUserID<< ", uuid: " << strUUID;
	}
}

CT_VOID CLoginServerThread::SetUserAccountInfo(const CT_CHAR* pAccountName, const UserAccountsBaseData& accountData)
{
	static acl::string key, attr1, attr2, attr3, attr4, attr5, attr6, attr7, attr8, attr9, attr10, attr11, attr12, attr13, attr14,
	attr15, attr16, attr17, attr18, attr19, attr20, attr21, attr22, /*attr23, attr24, attr25, attr26, attr27,*/ attr28, attr29;
	static acl::string val1, val2, val3, val4, val5, val6, val7, val8, val9,val10, val11, val12, val13, val14,
	val15, val16, val17, val18, val19, val20, val21, val22, /*val23, val24, val25, val26, val27,*/ val28, val29;
	static std::map<acl::string, acl::string> accountInfo;

	if (key.empty())
	{
		attr1.format("account");
		attr2.format("mobile");
		attr3.format("gender");
		attr4.format("nick");
		attr5.format("ip");
		attr6.format("location");
		attr7.format("headid");
		attr8.format("score");
		attr9.format("bankscore");
		attr10.format("bankpwd");
		attr11.format("pwd");
		attr12.format("alipay");
		attr13.format("alipayname");
		attr14.format("bankcard");
		attr15.format("bandname");
		attr16.format("vip");
		attr17.format("status");
		attr18.format("pay");
		attr19.format("channelid");
		attr20.format("c_cid");
		attr21.format("vip2");
		attr22.format("exchange");
		//attr23.format("zjhcount");
		//attr24.format("zjhparam");
		//attr25.format("qznncount");
		//attr26.format("qznnparam");
		//attr27.format("goodcardtime");
		attr28.format("rechargecount");
		attr29.format("pid");
	}
	key.format("account_%d", accountData.dwUserID);

	val1.format("%s", pAccountName);
	val2.format("%s", accountData.szMobileNum);
	val3.format("%d", accountData.cbGender);
	val4.format("%s", accountData.szNickName);
	val5.format("%s", accountData.szIP);
	val6.format("%s", accountData.szLocation);
	val7.format("%d", accountData.cbHeadId);
	val8.format("%lld", accountData.llScore);
	val9.format("%lld", accountData.llBankScore);
	val10.format("%s", accountData.szBankPassword);
	val11.format("%s", accountData.szPassword);
	val12.format("%s", accountData.szAliPayAccout);
	val13.format("%s", accountData.szAliPayName);
	val14.format("%s", accountData.szBankCardNum);
	val15.format("%s", accountData.szBankCardName);
	val16.format("%d", accountData.cbVipLevel);
	val17.format("%d", en_Account_Normal);
	val18.format("%u", accountData.dwTotalRecharge);
	val19.format("%d", accountData.dwChannelID);
	val20.format("%d", accountData.dwClientChannelID);
	val21.format("%d", accountData.cbVipLevel2);
	val22.format("%d", accountData.cbShowExchange);
	//val23.format("%u", accountData.dwZjhCount);
	//val24.format("%d", accountData.cbZjhGoodCardParam);
	//val25.format("%u", accountData.dwQznnCount);
	//val26.format("%d", accountData.cbQznnGoodCardParam);
	//val27.format("%u", accountData.dwGoodCardTime);
	val28.format("%d", 0);
	val29.format("%d", accountData.cbPlatformID);

	accountInfo[attr1] = val1;
	accountInfo[attr2] = val2;
	accountInfo[attr3] = val3;
	accountInfo[attr4] = val4;
	accountInfo[attr5] = val5;
	accountInfo[attr6] = val6;
	accountInfo[attr7] = val7;
	accountInfo[attr8] = val8;
	accountInfo[attr9] = val9;
	accountInfo[attr10] = val10;
	accountInfo[attr11] = val11;
	accountInfo[attr12] = val12;
	accountInfo[attr13] = val13;
	accountInfo[attr14] = val14;
	accountInfo[attr15] = val15;
	accountInfo[attr16] = val16;
	accountInfo[attr17] = val17;
	accountInfo[attr18] = val18;
	accountInfo[attr19] = val19;
	accountInfo[attr20] = val20;
	accountInfo[attr21] = val21;
	accountInfo[attr22] = val22;
	//accountInfo[attr23] = val23;
	//accountInfo[attr24] = val24;
	//accountInfo[attr25] = val25;
	//accountInfo[attr26] = val26;
	//accountInfo[attr27] = val27;
    accountInfo[attr28] = val28;
    accountInfo[attr29] = val29;

	m_redis.clear();
	if (m_redis.hmset(key.c_str(), accountInfo) == false)
	{
		LOG(WARNING) << "set user account data fail, userid = " << accountData.dwUserID;
		return;
	}

	//LOG(WARNING) << "set login user redis pwd: " << accountData.szPassword;
}

CT_VOID CLoginServerThread::SetAccountUserID(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "p"<< (int)cbPlatformId << "_" << pAccountName;

	std::stringstream strVal;
	strVal << dwUserID;

	m_redis.clear();
	if (m_redis.set(strKey.str().c_str(), strVal.str().c_str()) == false)
	{
		LOG(WARNING) << "set user id failed, account: " << pAccountName;
	}
}

CT_VOID CLoginServerThread::SetUserBindMobileInfo(CT_DWORD dwUserID, const CT_CHAR* pNewAccountName, const CT_CHAR* pNickName, const CT_CHAR* pPassword)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string accountValue(pNewAccountName);
	acl::string pwdValue(pPassword);

	std::map<acl::string, acl::string> mapValue;
	mapValue["account"] = accountValue;
	mapValue["mobile"] = accountValue;
	mapValue["pwd"] = pwdValue;
	mapValue["nick"] = acl::string(pNickName);

	//LOG(WARNING) << "SetUserBindMobileInfo, user id " << dwUserID << "user pwd: " << pwdValue.c_str();

	m_redis.clear();
	if (m_redis.hmset(key.c_str(), mapValue) == false)
	{
		LOG(WARNING) << "set user pwd and account fail. account: " << pNewAccountName;
	}
}

CT_VOID CLoginServerThread::SetUserPwd(const CT_CHAR* pNewAccountName, const CT_CHAR* pPassword, CT_DWORD& dwUserID, CT_BYTE cbPlatformId)
{
	acl::string key;
	key.format("p%d_%s", cbPlatformId, pNewAccountName);

	acl::string userid;
	m_redis.clear();
	if (!m_redis.get(key.c_str(), userid))
	{
		return;
	}

	if (userid.empty())
	{
		return;
	}

	dwUserID = atoi(userid.c_str());

	key.format("account_%s", userid.c_str());
	m_redis.clear();
	if (m_redis.hset(key.c_str(), "pwd", pPassword) < 0)
	{
		LOG(WARNING) << "set user pwd fail. account: " << pNewAccountName;
	}

	//LOG(WARNING) << "SetUserPwd, userid:  " << userid.c_str() << ", set pwd: " << pPassword;
}

CT_VOID CLoginServerThread::SetUserBankPwd(CT_DWORD dwUserID, const CT_CHAR* pPassword)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hset(key.c_str(), "bankpwd", pPassword) < 0)
	{
		LOG(WARNING) << "set user bank pwd fail. user id: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::RenameAccountUserID(const CT_CHAR* pOldAccountName, const CT_CHAR* pNewAccountName, CT_BYTE cbPlatformId)
{
	acl::string keyOld;
	keyOld.format("p%d_%s", cbPlatformId, pOldAccountName);

	acl::string keyNew;
	keyNew.format("p%d_%s", cbPlatformId, pNewAccountName);

	m_redis.clear();
	if (m_redis.rename_key(keyOld.c_str(), keyNew.c_str()) == false)
	{
		LOG(WARNING) << "rename userid account fail. account: " << pNewAccountName;
	}
}

CT_VOID CLoginServerThread::SetUserPersonalInfo(MSG_CS_Modify_PersonalInfo* pPersonalInfo)
{
	acl::string key;
	key.format("account_%u", pPersonalInfo->dwUserID);

	acl::string gender;
	acl::string headid;
	gender.format("%d", pPersonalInfo->cbSex);
	headid.format("%d", pPersonalInfo->cbHeadId);

	std::map<acl::string, acl::string> mapValue;
	mapValue["gender"] = gender;
	mapValue["headid"] = headid;

	m_redis.clear();
	if (m_redis.hmset(key.c_str(), mapValue) == false)
	{
		LOG(WARNING) << "update user personal info fail. userid: " << pPersonalInfo->dwUserID;
	}
}


CT_VOID	CLoginServerThread::SetProxyServerInfo(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, CT_DWORD dwUserID, const CT_CHAR* szMachineSerial)
{
	acl::string key;
	acl::string psid, psidVal;
	acl::string serial, serialVal;
	//acl::string kindid, kindidVal;
	acl::string clientNet, clientNetVal;
	std::map<acl::string, acl::string> psInfo;

	key.format("psinfo_%u", dwUserID);
	psid.format("psid");
    serial.format("serial");
	//kindid.format("kindid");
	clientNet.format("netaddr");

	m_redis.clear();
	std::map<acl::string, acl::string> psInfoOld;
	if (m_redis.hgetall(key, psInfoOld) == true)
	{
		//原来的ps信息不为空
		if (!psInfoOld.empty())
		{
		    //判断是否重新登陆
		    acl::string strOldSerial = psInfoOld["serial"];
		    CT_BOOL bSendToClient = false;
		    //两次机器码登陆
		    if (strcmp(strOldSerial.c_str(), szMachineSerial) != 0)
			{
                bSendToClient = true;
			}

			CT_DWORD dwPsID = atoi(psInfoOld["psid"]);
			CT_UINT64 uClientAddr = Utility::stingToUint64(psInfoOld["netaddr"].c_str());
			acl::aio_socket_stream* pPsSocket = CServerMgr::get_instance().FindProxyServer(dwPsID);
			if (pPsSocket)
			{
				SendKickUserMsg(pPsSocket, uClientAddr, dwUserID, bSendToClient);
			}
		}
	}

	// 设置新值
	CT_DWORD dwPsID = CServerMgr::get_instance().FindProxyServerID(pSocket);
	psidVal.format("%u", dwPsID);
    serialVal.format("%s", szMachineSerial);
	//kindidVal.format("%u", dwKindID);
#ifdef _OS_WIN32_CODE
	clientNetVal.format("%llu", uClientValue);
#elif defined(_OS_LINUX_CODE)
	clientNetVal.format("%lu", uClientValue);
#else
	clientNetVal.format("%llu", uClientValue);
#endif // !_OS_WIN32_CODE
	
	psInfo[psid] = psidVal;
	psInfo[serial] = serialVal;
	//psInfo[kindid] = kindidVal;
	psInfo[clientNet] = clientNetVal;

	m_redis.clear();
	if (m_redis.hmset(key, psInfo) == false)
	{
		LOG(WARNING) << "set proxy server info fail, userid = " << dwUserID;
		return;
	}

	return;
}

CT_VOID CLoginServerThread::SetUserGameRound(CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo)
{
	std::stringstream strKey;
	strKey << "gameround_" << dwUserID;

	std::map<acl::string, acl::string> gameRoundInfo;
	gameRoundInfo["0:0"] = "0";

	for (auto it : vecGameRoundInfo)
	{
		acl::string att;
		acl::string val;

		att.format("%u:%u", it.wGameID, it.wKindID);
		val.format("%u", it.dwGameRoundCount);
		gameRoundInfo[att] = val;
	}
	m_redis.clear();
	if (m_redis.hmset(strKey.str().c_str(), gameRoundInfo) == false)
	{
		LOG(WARNING) << "set game round count failed, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetUserSignIn(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "signin_" << dwUserID;

	std::map<acl::string, acl::string> benefitInfo;
	acl::string attCount, attTime;
	acl::string valCount, valTime;

	attCount.format("%s", "count");
	valCount.format("%d", m_signInData.bFirstSignIn);
	attTime.format("%s", "lastTime");
	//签到系统
	//valTime.format("%s", m_benefitData.strLastTime.c_str());

	benefitInfo[attCount] = valCount;
	benefitInfo[attTime] = valTime;

	m_redis.clear();
	if (m_redis.hmset(strKey.str().c_str(), benefitInfo) == false)
	{
		LOG(WARNING) << "set benefit failed, userid: " << dwUserID;
	}
}

CT_VOID	CLoginServerThread::SetUserBenefitInfo(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "account_" << dwUserID;

	std::map<acl::string, acl::string> benefitInfo;
	acl::string attCount, attTime;
	acl::string valCount, valTime;

	attCount.format("%s", "almsc");
	valCount.format("%d", m_benefitData.cbCurrCount);
	attTime.format("%s", "almstime");
	valTime.format("%u", m_benefitData.dwLastTime);

	benefitInfo[attCount] = valCount;
	benefitInfo[attTime] = valTime;

	m_redis.clear();
	if (m_redis.hmset(strKey.str().c_str(), benefitInfo) == false)
	{
		LOG(WARNING) << "set benefit failed, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetUserPlayData(CT_DWORD dwUserID,
											std::map<CT_DWORD, std::unique_ptr<stUserPlayData>> &mapUserPlayData)
{
	std::stringstream strKey;
	strKey << "account_" << dwUserID;

	std::map<acl::string, acl::string> playCountInfo;
	for (auto& it : mapUserPlayData)
	{
		acl::string attr;
		acl::string value;

		attr.format("%u_c", it.first);
		value.format("%u", it.second->dwPlayCount);
		playCountInfo[attr] = value;

		attr.format("%u_s", it.first);
		value.format("%lld", it.second->llWinScore);
		playCountInfo[attr] = value;
	}

	m_redis.clear();
	if (m_redis.hmset(strKey.str().c_str(), playCountInfo) == false)
	{
		LOG(WARNING) << "SetUserPlayData failed, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetUserScoreInfo(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "scoreinfo_" << dwUserID;

	std::map<acl::string, acl::string> scoreInfo;
	acl::string strRevenue, strWinCount, strLostCount, strDrawCount, strFleeCount, strPlayTime;

	strRevenue.format("%u", m_scoreData.dwRevenue);
	strWinCount.format("%u", m_scoreData.dwWinCount);
	strLostCount.format("%u", m_scoreData.dwLostCount);
	strDrawCount.format("%u", m_scoreData.dwDrawCount);
	strFleeCount.format("%u", m_scoreData.dwFleeCount);
	strPlayTime.format("%u", m_scoreData.dwPlayTime);

	scoreInfo["revenue"] = strRevenue;
	scoreInfo["winCount"] = strWinCount;
	scoreInfo["lostCount"] = strLostCount;
	scoreInfo["drawCount"] = strDrawCount;
	scoreInfo["fleeCount"] = strFleeCount;
	scoreInfo["playTime"] = strPlayTime;

	m_redis.clear();
	if (m_redis.hmset(strKey.str().c_str(), scoreInfo) == false)
	{
		LOG(WARNING) << "set user score info failed, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetUserBindPromoterID(CT_DWORD dwUserID, CT_DWORD dwPromoterID)
{
	acl::string key;
	key.format("account_%u", dwUserID);
	acl::string val;
	val.format("%u", dwPromoterID);

	m_redis.clear();
	if (m_redis.hset(key, "bindpromoterid", val.c_str()) == -1)
	{
		LOG(WARNING) << "update user bind promoter id fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserTaskInfo(CT_DWORD dwUserID)
{
	if (m_vecTask.empty())
	{
		return;
	}

	acl::string key;
	key.format("task_%u", dwUserID);
	
	acl::string attr;
	acl::string val; 
	std::map<acl::string, acl::string> mapTaskInfo;
	for (auto it = m_vecTask.begin(); it != m_vecTask.end(); ++it)
	{
		val.format("%u,%u,%u,%d", it->wTaskID, it->dwCurrProgress, it->dwReceiveTime, it->bFinish);
		attr.format("%u", it->wTaskID);
		mapTaskInfo[attr] = val;
	}

	m_redis.clear();
	if (m_redis.hmset(key, mapTaskInfo) == false)
	{
		LOG(WARNING) << "set user task fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserEarnScore(CT_DWORD dwUserID, CT_LONGLONG llEarnScore)
{
	acl::string key;
	key.format("earnscore_%u", dwUserID);

	acl::string attr;
	attr.format("%lld", llEarnScore);

	m_redis.clear();
	if (m_redis.set(key.c_str(), attr.c_str()) == false)
	{
		LOG(WARNING) << "set user earn score fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserAliPay(MSG_CS_Bind_AliPay* pBindAliPay)
{
	acl::string key;
	key.format("account_%u", pBindAliPay->dwUserID);

	acl::string aliPayAccount;
	acl::string aliPayName;
	aliPayAccount.format("%s", pBindAliPay->szAliPayAccount);
	aliPayName.format("%s", pBindAliPay->szRealName);

	std::map<acl::string, acl::string> mapAliPayInfo;
	mapAliPayInfo["alipay"] = aliPayAccount;
	mapAliPayInfo["alipayname"] = aliPayName;

	m_redis.clear();
	if (m_redis.hmset(key, mapAliPayInfo) == false)
	{
		LOG(WARNING) << "update alipay fail, user id: " << pBindAliPay->dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserBankCard(MSG_CS_Bind_BankCard* pBankCard)
{
	acl::string key;
	key.format("account_%u", pBankCard->dwUserID);

	acl::string bankCard;
	acl::string bankAccount;
	bankCard.format("%s", pBankCard->szBankCard);
	bankAccount.format("%s", pBankCard->szRealName);

	std::map<acl::string, acl::string> mapBankCardInfo;
	mapBankCardInfo["bankcard"] = bankCard;
	mapBankCardInfo["bandname"] = bankAccount;

	m_redis.clear();
	if (m_redis.hmset(key, mapBankCardInfo) == false)
	{
		LOG(WARNING) << "update bank card fail, user id: " << pBankCard->dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum, CT_DWORD dwVerifyCode)
{
	acl::string key;
	key.format("p%d_verify_%s", cbPlatformId, pMobileNum);

	acl::string attr;
	attr.format("%d", dwVerifyCode);

	m_redis.clear();
	if (m_redis.set(key.c_str(), attr.c_str()) == false)
	{
		LOG(WARNING) << "set mobile verify code fail, mobile num: " << pMobileNum;
		return;
	}
	//设置超时时间
	m_redis.clear();
	m_redis.pexpire(key.c_str(), 6*60*1000);
}

CT_VOID CLoginServerThread::SetUserLocationAndIp(CT_DWORD dwUserID, const CT_CHAR* pLocation, const CT_CHAR* pIp)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string strLocation;
	acl::string strIp;
	strLocation.format("%s", pLocation);
	strIp.format("%s", pIp);

	std::map<acl::string, acl::string> mapLocation;
	mapLocation["location"] = strLocation;
	mapLocation["ip"] = strIp;

	m_redis.clear();
	if (m_redis.hmset(key, mapLocation) == false)
	{
		LOG(WARNING) << "set user location and ip fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserTodayRankInfo(CT_DWORD dwUserID, CT_DWORD dwTodayOnline, CT_WORD wTodayTaskFinish, CT_DWORD dwTodayTaskReward)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string strOnline;
	acl::string strTaskFinish;
	acl::string strTaskReward;
	strOnline.format("%d", dwTodayOnline);
	strTaskFinish.format("%d", wTodayTaskFinish);
	strTaskReward.format("%d", dwTodayTaskReward);

	std::map<acl::string, acl::string> mapToday;
	mapToday["online"] = dwTodayOnline;
	mapToday["taskfinish"] = wTodayTaskFinish;
	mapToday["taskscore"] = dwTodayTaskReward;
	m_redis.clear();
	if (m_redis.hmset(key, mapToday) == false)
	{
		LOG(WARNING) << "set user today rank info fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::SetUserGoodCardInfo(UserAccountsBaseData& accountData)
{
	//计算是否已经跨天
	/*CT_DWORD nTimeNow = time(NULL);
	CT_BOOL bAcrossDay = isAcrossTheDay(accountData.dwGoodCardTime, nTimeNow);
	if (bAcrossDay)
	{
		acl::string key;
		key.format("account_%u", accountData.dwUserID);

		//acl::string strZjhCount;
		acl::string strZjhParam;
		//acl::string strQznnCount;
		acl::string strQznnParam;
		acl::string strGoodCardTime;

		accountData.dwGoodCardTime = nTimeNow;
		if (accountData.dwQznnCount < 3)
		{
			accountData.cbQznnGoodCardParam = 80;
		}
		else
		{
			accountData.cbQznnGoodCardParam = 50 + (rand()%5)*10;
		}

		if (accountData.dwZjhCount < 3)
		{
			accountData.cbZjhGoodCardParam = 80;
		}
		else
		{
			accountData.cbZjhGoodCardParam = 50 + (rand()%5)*10;
		}

		strZjhParam.format("%d", accountData.cbZjhGoodCardParam);
		strQznnParam.format("%d", accountData.cbQznnGoodCardParam);
		strGoodCardTime.format("%u", accountData.dwGoodCardTime);

		std::map<acl::string, acl::string> mapGoodCard;
		mapGoodCard["zjhparam"] =  strZjhParam;
		mapGoodCard["qznnparam"] = strQznnParam;
		mapGoodCard["goodcardtime"] = strGoodCardTime;
		//LOG(WARNING) << "set good card info!";

		m_redis.clear();
		if (m_redis.hmset(key, mapGoodCard) == false)
		{
			LOG(WARNING) << "set user good card info fail, user id: " << accountData.dwUserID;
			return;
		}

		//更新数据库
		UpdateUserGoodCardInfo(accountData);
	}*/
}

CT_VOID CLoginServerThread::SetRedisKeyPersist(acl::string key)
{
	//设置无限生命周期
	m_redis.clear();
	int nResult = m_redis.persist(key.c_str());
	if (nResult < 0)
	{
		LOG(WARNING) << "set key persist fail, key: " << key.c_str();
	}
}

CT_BOOL CLoginServerThread::GetAccountInfo(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, UserAccountsBaseData& accountData)
{
	acl::string key;
	acl::string strUserID;
	key.format("p%d_%s", cbPlatformId, pAccountName);

	m_redis.clear();
	if (m_redis.get(key.c_str(), strUserID) == false)
	{
		return false;
	}

	if (strUserID.empty())
	{
		return false;
	}

	std::map<acl::string, acl::string> mapAccountInfo;
	key.format("account_%s", strUserID.c_str());

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), mapAccountInfo) == false)
	{
		return false;
	}

	if (mapAccountInfo.empty())
	{
		return false;
	}

	accountData.dwUserID = (CT_DWORD)atoi(strUserID.c_str());
	accountData.llScore = atoll(mapAccountInfo["score"]);
	accountData.llBankScore = atoll(mapAccountInfo["bankscore"]);
	accountData.cbVipLevel = (CT_BYTE)atoi(mapAccountInfo["vip"]);
	accountData.cbVipLevel2 = (CT_BYTE)atoi(mapAccountInfo["vip2"]);
	accountData.dwTotalRecharge = (CT_DWORD)atoi(mapAccountInfo["pay"]);
	accountData.cbGender = (CT_BYTE)atoi(mapAccountInfo["gender"]);
	accountData.cbHeadId = (CT_BYTE)atoi(mapAccountInfo["headid"]);
	accountData.cbStatus = (CT_BYTE)atoi(mapAccountInfo["status"]);
	accountData.dwChannelID = (CT_DWORD)atoi(mapAccountInfo["channelid"]);
	accountData.cbShowExchange = (CT_BYTE)atoi(mapAccountInfo["exchange"]);
	accountData.cbPlatformID = (CT_BYTE)atoi(mapAccountInfo["pid"]);
	m_benefitData.cbVip2 = accountData.cbVipLevel2;
	m_benefitData.cbCurrCount = (CT_BYTE)atoi(mapAccountInfo["almsc"]);
	m_benefitData.dwLastTime = (CT_DWORD)atoi(mapAccountInfo["almstime"]);

    //炸金花和抢庄牛牛的数据
	//accountData.dwZjhCount = atoi(mapAccountInfo["zjhcount"]);
	//accountData.cbZjhGoodCardParam = (CT_BYTE)atoi(mapAccountInfo["zjhparam"]);
	//accountData.dwQznnCount = atoi(mapAccountInfo["qznncount"]);
	//accountData.cbQznnGoodCardParam = (CT_BYTE)atoi(mapAccountInfo["qznnparam"]);
	//accountData.dwGoodCardTime = atoi(mapAccountInfo["goodcardtime"]);

	_snprintf_info(accountData.szMobileNum, sizeof(accountData.szMobileNum), "%s", mapAccountInfo["mobile"].c_str());
	_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", mapAccountInfo["nick"].c_str());
	//_snprintf_info(accountData.szHeadUrl, sizeof(accountData.szHeadUrl), "%s", mapAccountInfo["head"].c_str());
	_snprintf_info(accountData.szPassword, sizeof(accountData.szPassword), "%s", mapAccountInfo["pwd"].c_str());
	_snprintf_info(accountData.szBankPassword, sizeof(accountData.szBankPassword), "%s", mapAccountInfo["bankpwd"].c_str());
	//兑换帐号信息
	_snprintf_info(accountData.szAliPayAccout, sizeof(accountData.szAliPayAccout), "%s", mapAccountInfo["alipay"].c_str());
	_snprintf_info(accountData.szAliPayName, sizeof(accountData.szAliPayName), "%s", mapAccountInfo["alipayname"].c_str());
	_snprintf_info(accountData.szBankCardNum, sizeof(accountData.szBankCardNum), "%s", mapAccountInfo["bankcard"].c_str());
	_snprintf_info(accountData.szBankCardName, sizeof(accountData.szBankCardName), "%s", mapAccountInfo["bandname"].c_str());
	return true;
}

CT_LONGLONG CLoginServerThread::GetUserScore(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string score;
	m_redis.clear();
	if (m_redis.hget(key.c_str(), "score",  score) == false)
	{
        LOG(WARNING) << "get user score fail,userid  " << dwUserID;
		return 0;
	}

	return atoll(score.c_str());
}

CT_LONGLONG CLoginServerThread::GetUserBankScore(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string score;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "bankscore", score) == false)
	{
		return 0;
	}

	return atoll(score.c_str());
}

CT_DWORD CLoginServerThread::GetUserGem(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string gem;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "gem", gem) == false)
	{
		return 0;
	}

	return atoi(gem.c_str());
}

CT_BYTE	CLoginServerThread::GetUserVipLevel(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string vip;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "vip", vip) == false)
	{
		return 0;
	}

	return atoi(vip.c_str());
}

CT_BYTE  CLoginServerThread::GetUserVip2Level(CT_DWORD dwUserID)
{
    acl::string key;
    key.format("account_%u", dwUserID);

    acl::string vip2;

    m_redis.clear();
    if (m_redis.hget(key.c_str(), "vip2", vip2) == false)
    {
        return 0;
    }

    return atoi(vip2.c_str());
}

CT_WORD CLoginServerThread::GetUserChannelID(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string channnelID;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "channelid", channnelID) == false)
	{
		return 0;
	}

	return atoi(channnelID.c_str());
}

CT_BOOL CLoginServerThread::CheckUserOtherGameOnline(CT_DWORD dwUserID, CT_DWORD wGameID, std::map<acl::string, acl::string>& mapOnlineInfo)
{
	acl::string key;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		return false;
	}

	m_redis.clear();
	if (m_redis.hgetall(key, mapOnlineInfo) == false)
	{
		return false;
	}

	/*CT_DWORD wOldGameID = atoi(mapOnlineInfo["gameid"].c_str());
	if (wOldGameID != wGameID)
	{
		return true;
	}*/

	return true;
}

CT_BOOL CLoginServerThread::CheckUserInGameOnline(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		return false;
	}

	return true;
}

CT_BOOL CLoginServerThread::IsExistUserSession(CT_DWORD dwUserID, const CT_CHAR* pSession)
{
	acl::string key, val;
	key.format("session_%u", dwUserID);

	m_redis.clear();
	if (m_redis.get(key, val) != false)
	{
		if (val == pSession)
		{
			return true;
		}
		return false;
	}

	return false;
}

CT_VOID CLoginServerThread::AddUserGemAndScoreToCache(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore, CT_BOOL bUpdateToClient)
{
	acl::string key;
	key.format("account_%u", dwUserID);
	acl::string val;

	if (nAddGem != 0)
	{
		val.format("%d", nAddGem);
		m_redis.clear();

		long long llNewGem = 0;
		if (m_redis.hincrby(key, "gem", nAddGem, &llNewGem) == false)
		{
			LOG(WARNING) << "update redis gem fail, user id: " << dwUserID;
			return;
		}

		if (bUpdateToClient)
		{
			UpdateUserGemToClient(pSocket, dwUserID, nAddGem, (CT_DWORD)llNewGem);
		}

	}
	
	if (llAddScore != 0)
	{
		val.format("%lld", llAddScore);
		m_redis.clear();

		long long llNewScore = 0;
		if (m_redis.hincrby(key, "score", llAddScore, &llNewScore) == false)
		{
			LOG(WARNING) << "update redis score fail, user id: " << dwUserID;
			return;
		}

		if (bUpdateToClient)
		{
			UpdateUserScoreToClient(pSocket, dwUserID, llAddScore, llNewScore);
		}
	}
}

CT_VOID CLoginServerThread::AddUserBankScoreToCache(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llAddBankScore)
{
	acl::string key;
	key.format("account_%u", dwUserID);
	acl::string val;

	val.format("%lld", llAddBankScore);
	m_redis.clear();
	long long llNewBankScore = 0;
	if (m_redis.hincrby(key, "bankscore", llAddBankScore, &llNewBankScore) == false)
	{
		LOG(WARNING) << "update redis bank score fail, user id: " << dwUserID;
		return;
	}

	UpdateUserBankScoreToClient(pSocket, dwUserID, llNewBankScore);
}

CT_VOID CLoginServerThread::AddUserGemAndScoreToCenterServer(CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore)
{
	if (nAddGem != 0)
	{
		MSG_UpdateUser_Gem updateUserGem;
		updateUserGem.dwUserID = dwUserID;
		updateUserGem.nAddGem = nAddGem;
		updateUserGem.bUpdateToClient = false;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_LCS_MAIN, SUB_L2CS_ADD_USER_GEM, &updateUserGem, sizeof(updateUserGem));
	}

	if (llAddScore != 0)
	{
		MSG_UpdateUser_Score updateUserScore;
		updateUserScore.dwUserID = dwUserID;
		updateUserScore.llAddScore = llAddScore;
		updateUserScore.bUpdateToClient = false;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_LCS_MAIN, SUB_L2CS_ADD_USER_SCORE, &updateUserScore, sizeof(updateUserScore));
	}
}

CT_VOID CLoginServerThread::AddBankScoreToCenterServer(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore)
{
	MSG_UpdateUser_BankScore updateUserBankScore;
	updateUserBankScore.dwUserID = dwUserID;
	updateUserBankScore.llAddBankScore = llAddBankScore;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_LCS_MAIN, SUB_L2CS_ADD_USER_BANKSCORE, &updateUserBankScore, sizeof(updateUserBankScore));
}

CT_BOOL	CLoginServerThread::GetUserBenefitInfo(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

    acl::string att_vip2, att_benefitCount, att_benefitTime;
    const char* attrs[3];
    std::vector<acl::string> result;
	att_vip2.format("vip2");
    att_benefitCount.format("almsc");
    att_benefitTime.format("almstime");

    attrs[0] = att_benefitCount.c_str();
    attrs[1] = att_benefitTime.c_str();
	attrs[2] = att_vip2.c_str();

    m_redis.clear();
    if (m_redis.hmget(key.c_str(), attrs, 3, &result) == false)
    {
        LOG(WARNING) << "get user benefit info fail, user id: " << dwUserID;
        return false;
    }

    if (result.size() != 3)
    {
        return false;
    }

	m_redis.clear();
	if (m_redis.hmget(key, result) == false)
	{
		return false;
	}

	m_benefitData.cbCurrCount = (CT_BYTE)atoi(result[0].c_str());
	m_benefitData.dwLastTime = (CT_DWORD)atoi(result[1].c_str());
	m_benefitData.cbVip2 = (CT_BYTE)atoi(result[2].c_str());

	CT_DWORD dwNow = time(NULL);
	bool bAcrossDay = isAcrossTheDay(m_benefitData.dwLastTime, dwNow);
	if (bAcrossDay)
	{
		m_benefitData.cbCurrCount = 0;
	}

	return true;
}

CT_BOOL CLoginServerThread::GetUserScoreInfo(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("scoreinfo_%u", dwUserID);

	std::map<acl::string, acl::string> result;
	m_redis.clear();
	if (m_redis.hgetall(key, result) == false)
	{
		return false;
	}

	m_scoreData.dwRevenue = atoi(result["revenue"].c_str());
	m_scoreData.dwWinCount = atoi(result["winCount"].c_str());
	m_scoreData.dwLostCount = atoi(result["lostCount"].c_str());
	m_scoreData.dwDrawCount = atoi(result["drawCount"].c_str());
	m_scoreData.dwFleeCount = atoi(result["fleeCount"].c_str());
	m_scoreData.dwPlayTime = atoi(result["playTime"].c_str());

	return true;
}

CT_DWORD CLoginServerThread::GetUserBindPromoterID(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string promoterid;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "bindpromoterid", promoterid) == false)
	{
		return 0;
	}

	return atoi(promoterid.c_str());
}

CT_BOOL CLoginServerThread::GetUserBankPwd(CT_DWORD dwUserID, std::string& strBankPwd)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	static acl::string bankpwd;

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "bankpwd", bankpwd) == true)
	{
		strBankPwd = bankpwd.c_str();
		return true;
	}

	LOG(WARNING) << "get user bank password fail, userid: " << dwUserID;
	return false;
}

CT_BOOL CLoginServerThread::GetMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum, CT_DWORD& dwVerifyCode)
{
	acl::string key;
	key.format("p%d_verify_%s", cbPlatformId, pMobileNum);

	acl::string verifyCode;

	m_redis.clear();
	if (m_redis.get(key.c_str(), verifyCode) == false)
	{
		return false;
	}

	if (verifyCode.empty())
		return false;

	dwVerifyCode = atoi(verifyCode.c_str());

	return true;
}

CT_BYTE CLoginServerThread::GetUserPlatformId(CT_DWORD dwUserID)
{
    acl::string key;
    key.format("account_%u", dwUserID);

    static acl::string platformId;

    m_redis.clear();
    if (m_redis.hget(key.c_str(), "pid", platformId) == true)
    {
        return (CT_BYTE)atoi(platformId.c_str());
    }

    return 0;
}

CT_VOID CLoginServerThread::DeleteMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum)
{
	acl::string key;
	key.format("p%d_verify_%s", cbPlatformId, pMobileNum);

	m_redis.clear();
	if (m_redis.del_one(key.c_str()) <= 0)
	{
		LOG(WARNING) << "delete mobile verify code fail. mobile num: " << pMobileNum;
	}
}

CT_VOID CLoginServerThread::GetUserAliPayAccount(CT_DWORD dwUserID, acl::string& strAliPay)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "alipay", strAliPay) == false)
	{
		LOG(WARNING) << "get user alipay fail, userid: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::GetUserBankCard(CT_DWORD dwUserID, acl::string& strBankCard)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "bankcard", strBankCard) == false)
	{
		LOG(WARNING) << "get user bank card num fail, userid: " << dwUserID;
		return;
	}
}

CT_VOID CLoginServerThread::GetUserMobileNum(CT_DWORD dwUserID, acl::string& strMobileNum)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hget(key.c_str(), "mobile", strMobileNum) == false)
	{
		LOG(WARNING) << "get user mobile num fail, userid: " << dwUserID;
		return;
	}
}


CT_VOID CLoginServerThread::DelUserSession(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "session_" << dwUserID;
	m_redis.clear();
	if (m_redis.del_one(strKey.str().c_str()) <= 0)
	{
		LOG(WARNING) << "delete user session fail, userid: " << dwUserID;
	}
}

CT_BOOL CLoginServerThread::DelProxyServerInfo(CT_DWORD dwUserID, CT_DWORD dwProxyServerID)
{
	acl::string key;
	acl::string result;

	key.format("psinfo_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hget(key, "psid", result) == false)
	{
		return false;
	}

	CT_DWORD dwRedisProxyServerID = atoi(result.c_str());
	if (dwRedisProxyServerID != dwProxyServerID)
	{
		return false;
	}
	 
	m_redis.clear();
	if (m_redis.del_one(key) <= 0)
	{
		LOG(WARNING) << "delete user proxy server info fail, userid: " << dwUserID;
	}

	return true;
}

CT_VOID CLoginServerThread::SetAccountInfoExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "account_" << dwUserID;

	//acl::string strAccountName;

    std::vector<acl::string> vecName;
    std::vector<acl::string> vecResult;
    vecName.push_back("account");
    vecName.push_back("pid");

    m_redis.clear();
	//if (m_redis.hget(strKey.str().c_str(), "account", strAccountName) == true)
    if (m_redis.hmget(strKey.str().c_str(), vecName, &vecResult) == true)
	{
        if (vecResult.size() == 2)
        {
            std::stringstream strAccountKey;
            strAccountKey << "p" << vecResult[1].c_str() << "_" << vecResult[0].c_str();
            if (m_redis.expire(strAccountKey.str().c_str(), 10800) <= 0)
            {
                LOG(WARNING) << "expire user account name fail, userid: " << dwUserID;
            }
        }
	}

	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire user account info fail, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetBenefitExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "benefit_" << dwUserID;

	/*m_redis.clear();
	if (m_redis.exists(strKey.str().c_str()) == false)
	{
		return;
	}*/

	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire benefit info fail, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetScoreInfoExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "scoreinfo_" << dwUserID;

	/*m_redis.clear();
	if (m_redis.exists(strKey.str().c_str()) == false)
	{
		return;
	}*/

	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire score info fail, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetGameRoundCountExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "gameround_" << dwUserID;
	m_redis.clear();

	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire game round count fail, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetTaskExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "task_" << dwUserID;
	m_redis.clear();

	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire task fail, userid: " << dwUserID;
	}
}

CT_VOID	CLoginServerThread::SetEarnScoreExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "earnscore_" << dwUserID;
	m_redis.clear();

	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire earn score fail, userid: " << dwUserID;
	}
}

CT_VOID CLoginServerThread::SetAccountRelatedPersist(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, CT_DWORD dwUserID)
{
	//把相关redis键设置为无限期
	acl::string strKey;
	strKey.format("account_%u", dwUserID);
	SetRedisKeyPersist(strKey);

	strKey.format("p%d_%s", cbPlatformId, pAccountName);
	SetRedisKeyPersist(strKey);

	strKey.format("earnscore_%u", dwUserID);
	SetRedisKeyPersist(strKey);

	strKey.format("task_%u", dwUserID);
	SetRedisKeyPersist(strKey);
}

CT_VOID CLoginServerThread::QuerySignInInfo(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	QuerySignInInfoIndb(db, dwUserID, m_signInData);

	pool->put(db);
}

CT_VOID CLoginServerThread::QuerySignInInfoIndb(acl::db_handle* db, CT_DWORD dwUserID, stSignInData& signInData)
{
	acl::query query;
	query.create_sql("select lastDateTime, seriesDate from record_signin  where userid = :userid")
		.set_format("userid", "%u", dwUserID);
	
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select sign in info fail, user id: " << dwUserID;
		return;
	}

	CT_BOOL bFirstSignIn = true;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];

		std::string strEndTime = (*row)["lastDateTime"];
		std::string strSeriesDate = (*row)["seriesDate"];

		bFirstSignIn = false;

		//最后签到时间
		std::string lastSignInTime = strEndTime.substr(0, 10);
		std::string strYesterday = Utility::YesterdayString();
		std::string strToday = Utility::GetTimeNowString("%Y-%m-%d");

		if (lastSignInTime == strYesterday) //昨天签的到
		{
			signInData.wSignInDays = atoi(strSeriesDate.c_str());
			signInData.bNewDate = true;
		}
		else if (lastSignInTime == strToday)//今天签的到
		{
			signInData.wSignInDays = atoi(strSeriesDate.c_str());;
			signInData.bNewDate = false;
		}
		else
		{
			signInData.wSignInDays = 0;
			signInData.bNewDate = true;
		}
	}

	if (bFirstSignIn)
	{
		signInData.wSignInDays = 0;
		signInData.bNewDate = true;
		signInData.bFirstSignIn = true;
	}

	db->free_result();
}

CT_VOID CLoginServerThread::UpdateSignInRecord(acl::db_handle* db, CT_DWORD dwUserID/*, CT_DWORD dwRewardGem, CT_DWORD dwRewardScore*/)
{
	if (m_signInData.bNewDate)
	{
		//更新玩家签到记录
		acl::query query;
		if (m_signInData.bFirstSignIn)
		{
			query.create_sql("insert into record_signin(userid, lastDateTime, seriesDate)  values(:userid, :lastDateTime, :seriesDate)")
				.set_format("userid", "%u", dwUserID)
				.set_date("lastDateTime", time(NULL))
				.set_format("seriesDate", "%d", m_signInData.wSignInDays + 1);
		}
		else
		{
			query.create_sql("update record_signin  set lastDateTime = :lastDateTime, seriesDate=:seriesDate where userid = :userid")
				.set_date("lastDateTime", time(NULL))
				.set_format("seriesDate", "%d", m_signInData.wSignInDays + 1)
				.set_format("userid", "%u", dwUserID);
		}

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update sign in info fail, user id: " << dwUserID;
			return;
		}

		db->free_result();

		/*query.reset();

		//记录玩家详细
		query.create_sql("insert record_signin_detail(userid, signInDate, rewardGem, rewardScore) values(:userid, :signInDate, :rewardGem, :rewardScore)")
			.set_format("userid", "%u", dwUserID)
			.set_date("signInDate", time(NULL))
			.set_format("rewardGem", "%u", dwRewardGem)
			.set_format("rewardScore", "%u", dwRewardScore);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert sign in detail info fail, user id: " << dwUserID;
			return;
		}

		db->free_result();*/
	}
}

CT_VOID CLoginServerThread::QueryBenefitInfo(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	QueryBenefitInfoIndb(db, dwUserID, m_benefitData);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryBenefitInfoIndb(acl::db_handle* db, CT_DWORD dwUserID, tagBenefitData& benefitData)
{
	acl::query query;
	query.create_sql("select * from record_benefit where userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select benefit info fail, user id: " << dwUserID;
		return;
	}

	CT_BOOL bFirstSignIn = true;
	//std::string nowTime = Utility::GetTimeNowString();
	CT_DWORD  dwNow = (CT_DWORD)time(NULL);
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];

		CT_DWORD dwLastTime = (CT_DWORD)atoi((*row)["lastTime"]);
		std::string strCount = (*row)["count"];

		bFirstSignIn = false;

		//最后签到时间
		//std::string lastBenefitDate = strEndTime.substr(0, 10);
		//std::string today = nowTime.substr(0, 10);

		bool bAcrossDay = isAcrossTheDay(dwLastTime, dwNow);

		//如果今天未领取
		if (bAcrossDay)
		{
			benefitData.cbCurrCount = 0;
			benefitData.dwLastTime = dwNow;
		}
		else
		{
			benefitData.cbCurrCount = (CT_BYTE)atoi(strCount.c_str());
			benefitData.dwLastTime = dwLastTime;;
		}
	}
    db->free_result();

	if (bFirstSignIn)
	{
		benefitData.cbCurrCount = 0;
		benefitData.dwLastTime = dwNow;
		InsertBenefitRecord(db, dwUserID, 0);
	}
}

CT_VOID CLoginServerThread::InsertBenefitRecord(acl::db_handle* db, CT_DWORD dwUserID, CT_LONGLONG llRewardScore)
{
	//更新玩家签到记录
	acl::query query;
	query.create_sql("insert into record_benefit(userid, count, lastTime)  values(:userid, :count, :lastTime) ON DUPLICATE KEY UPDATE count=:count, lastTime=:lastTime")
		.set_format("userid", "%u", dwUserID)
		.set_format("count", "%d", m_benefitData.cbCurrCount)
		.set_format("lastTime", "%u", m_benefitData.dwLastTime);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update benefit info fail, user id: " << dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();
		return;
	}

	if (llRewardScore != 0)
	{
		query.reset();
		query.create_sql("insert into record_benefit_detail(userid, vip2, rewardScore, rewardTime) values(:userid, :vip2, :rewardScore, :rewardTime)")
			.set_format("userid", "%u", dwUserID)
			.set_format("vip2", "%d", m_benefitData.cbVip2)
			.set_format("rewardScore", "%lld", llRewardScore)
			.set_format("rewardTime", "%u", m_benefitData.dwLastTime);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update benefit detail info fail, user id: " << dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();
			return;
		}
	}

	db->free_result();
}

CT_VOID CLoginServerThread::QueryGamePlayData(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetRecorddbPool();
   if (pool == NULL)
   {
       LOG(WARNING) << "get record db pool fail.";
       return;
   }

   acl::db_handle* db = pool->peek_open();
   if (db == NULL)
   {
       LOG(WARNING) << "get record db handle fail.";
       return;
   }

   do
   {
       acl::query query;
       query.create_sql(" SELECT gameIndex, sum(winCount) + sum(lostCount) as playCount FROM src_user_today "
						"WHERE writeDate = :today and (gameIndex = 50101 or gameIndex = 20104) and userid = :userid GROUP BY `gameIndex`")
       .set_date("today", time(NULL), "%Y-%m-%d")
       .set_format("userid", "%u", dwUserID);

       if (db->exec_select(query) == false)
       {
           LOG(WARNING) << "QueryGamePlayData info fail, user id : " << dwUserID << ", errno: " << db->get_errno() << ", error msg: " << db->get_error();
           break;
       }

       std::map<CT_DWORD, std::unique_ptr<stUserPlayData>> mapUserPlayData;
       for (size_t i = 0; i < db->length(); i++)
       {
           const acl::db_row* row = (*db)[i];
           CT_DWORD dwGameIndex = (CT_DWORD)atoi((*row)["gameIndex"]);
           CT_DWORD dwPlayCount = (CT_DWORD)atoi((*row)["playCount"]);

           auto it = mapUserPlayData.find(dwGameIndex);
           if (it == mapUserPlayData.end())
		   {
			   std::unique_ptr<stUserPlayData> userPlayData(new stUserPlayData());
			   userPlayData->dwPlayCount = dwPlayCount;
			   mapUserPlayData.insert(std::make_pair(dwGameIndex, std::move(userPlayData)));
		   }
       }
	   db->free_result();

       query.reset();

       //是否应该考虑用union把两个语句合并
       query.create_sql("SELECT gameIndex, sum(winScore) + sum(lostScore) as totalWin FROM `src_user_stat` WHERE (gameIndex = 50101) and userid = :userid GROUP BY `gameIndex` LIMIT 1 ")
       .set_format("userid", "%u", dwUserID);

	   if (db->exec_select(query) == false)
	   {
		   LOG(WARNING) << "QueryGamePlayData info fail, user id : " << dwUserID << ", errno: " << db->get_errno() << ", error msg: " << db->get_error();
		   break;
	   }

	   for (size_t i = 0; i < db->length(); i++)
	   {
		   const acl::db_row* row = (*db)[i];
		   CT_DWORD dwGameIndex = (CT_DWORD)atoi((*row)["gameIndex"]);
		   CT_LONGLONG llTotalWin = atoll((*row)["totalWin"]);

		   auto it = mapUserPlayData.find(dwGameIndex);
		   if (it == mapUserPlayData.end())
		   {
			   std::unique_ptr<stUserPlayData> userPlayData(new stUserPlayData());
			   userPlayData->dwPlayCount = llTotalWin;

			   mapUserPlayData.insert(std::make_pair(dwGameIndex, std::move(userPlayData)));
		   }
		   else
		   {
			   std::unique_ptr<stUserPlayData>& userPlayData = it->second;
			   userPlayData->llWinScore = llTotalWin;
		   }
	   }
	   db->free_result();

       if (!mapUserPlayData.empty())
	   {
		   SetUserPlayData(dwUserID, mapUserPlayData);
	   }


   }while (0);

   pool->put(db);
}


CT_VOID CLoginServerThread::AddUserGemAndScoreTodb(acl::db_handle* db, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore)
{
	acl::query query;
	query.create_sql("update userinfo set gem = gem + :gem, score = score + :score where userid = :userid LIMIT 1")
		.set_format("gem", "%d", nAddGem)
		.set_format("score", "%lld", llAddScore)
		.set_format("userid", "%u", dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "add db gem or score fail, add gem: " << nAddGem << ",  add score : "<< llAddScore <<", user id: " << dwUserID;
		return;
	}

	db->free_result();
}

CT_VOID CLoginServerThread::AddUserBankScoreTodb(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("UPDATE userinfo SET bankscore = bankscore + :bankscore WHERE userid = :userid  LIMIT 1")
			.set_format("bankscore", "%lld", llAddBankScore)
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user bank score fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::InsertScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBank, CT_LONGLONG llSourceScore, CT_LONGLONG llAddBank, CT_LONGLONG llAddScore, CT_BYTE cbType, CT_DWORD dwMailID/* = 0*/)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	std::string date(Utility::GetTimeNowString());
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string scoreChangeTable("insert into record_score_change_");
	scoreChangeTable.append(year);
	scoreChangeTable.append(month);
	do
	{
		//values(:userid, :sourceBank, :sourceScore, :changeBank, :changeScore, :type, UNIX_TIMESTAMP(NOW()))")
		std::string scoreChangeSql(scoreChangeTable);
		acl::query query;
		query.create_sql(" (userid, sourceBank, sourceScore, changeBank, changeScore, drawid, type, fixdate) \
				 values(:userid, :sourceBank, :sourceScore, :changeBank, :changeScore, :drawid, :type, :time)")
			.set_format("userid", "%u", dwUserID)
			.set_format("sourceBank", "%lld", llSourceBank)
			.set_format("sourceScore", "%lld", llSourceScore)
			.set_format("changeBank", "%lld", llAddBank)
			.set_format("changeScore", "%lld", llAddScore)
            .set_format("drawid", "%u", dwMailID)
			.set_format("type", "%d", cbType)
			.set_format("time", "%d", (CT_DWORD)time(NULL));

		scoreChangeSql.append(query.to_string());
		if (db->sql_update(scoreChangeSql.c_str()) == false)
		{
			LOG(WARNING) << "update score change info fail, user id: " << dwUserID << ", type: " << (int)cbType << ", change score: " << llAddScore << ", errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryUserGameRound(CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	QueryUserGameRound(db, dwUserID, vecGameRoundInfo);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryUserGameRound(acl::db_handle* db, CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo)
{
	acl::query query;
	query.create_sql("select * from record_user_gameround where userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query user game round info fail, user id: " << dwUserID;
		return;
	}

	stGameRoundInfo gameRoundInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		gameRoundInfo.wGameID = (CT_WORD)atoi((*row)["gameid"]);
		gameRoundInfo.wKindID = (CT_WORD)atoi((*row)["kindid"]);
		gameRoundInfo.dwGameRoundCount = (CT_DWORD)atoi((*row)["gameRoundCount"]);
		vecGameRoundInfo.push_back(gameRoundInfo);
	}

	db->free_result();
}


CT_VOID CLoginServerThread::QueryUserMail(CT_DWORD dwUserID, CT_DWORD dwChannelID, std::vector<MSG_SC_Mail_Info>& vecUserMail)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	QueryUserMailIndb(db, dwUserID, dwChannelID, vecUserMail);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryUserMailIndb(acl::db_handle* db, CT_DWORD dwUserID, CT_DWORD dwChannelID, std::vector<MSG_SC_Mail_Info>& vecUserMail)
{
	vecUserMail.clear();
	
	acl::query query;
	//先把过期的邮件写进mail_del
	query.create_sql("select mailid from mail where userid = :userid and mailType = 2 and expireTime < now()")
		.set_format("userid", "%u", dwUserID);
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select expire mail fail, error no: " << db->get_errno() << ", sql:" << query.to_string().c_str();
		return;
	}

	std::vector<CT_DWORD> vecMailID;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_DWORD dwMailID = (CT_DWORD)atoi((*row)["mailid"]);
		vecMailID.push_back(dwMailID);
	}
	
	db->free_result();
	for (auto& it : vecMailID)
	{
		query.reset();
		query.create_sql("insert into mail_del select *, now() from mail where mailid=:mailid")
			.set_format("mailid", "%u", it);
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert mail_del fail: " << query.to_string().c_str();
			return;
		}
		db->free_result();
	}


	//再把过期的邮件从mail删除
	query.reset();
	query.create_sql("delete from mail where userid = :userid and mailType = 2 and expireTime < now()")
		.set_format("userid", "%u", dwUserID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete user game mail fail: " << query.to_string().c_str();
		return;
	}
	db->free_result();


	query.reset();
	query.create_sql("select * from mail where userid = :userid or (userid = 0 and (channelid = :channelid or channelid = 0)) ORDER BY mailid DESC limit 10")
		.set_format("userid", "%u", dwUserID)
		.set_format("channelid", "%u", dwChannelID);
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query user game mail fail: " << query.to_string().c_str();
		return;
	}

	MSG_SC_Mail_InfoEx mailInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		mailInfo.dwMailID = (CT_DWORD)atoi((*row)["mailid"]);
		mailInfo.dwUserID = (CT_DWORD)atoi((*row)["userid"]);
		std::string strTitle = (*row)["title"];
		std::string strContent = (*row)["content"];
		_snprintf_info(mailInfo.szTitle, sizeof(mailInfo.szTitle), "%s", Utility::Ansi2Utf8(strTitle.c_str()).c_str());
		_snprintf_info(mailInfo.szContent, sizeof(mailInfo.szContent), "%s", Utility::Ansi2Utf8(strContent.c_str()).c_str());
		//mailInfo.wGem = (CT_WORD)atoi((*row)["gem"]);
		mailInfo.dScore = atoll((*row)["score"])*TO_DOUBLE;
		mailInfo.cbMailType = (CT_BYTE)atoi((*row)["mailType"]);
		mailInfo.cbState = (CT_BYTE)atoi((*row)["state"]);
		_snprintf_info(mailInfo.szTime, sizeof(mailInfo.szTime), "%s", Utility::Ansi2Utf8((*row)["sendTime"]).substr(0, 10).c_str());
		vecUserMail.push_back(mailInfo);
	}

	db->free_result();
}

CT_BOOL CLoginServerThread::QueryUserOneMailIndb(acl::db_handle* db, CT_DWORD dwMailID, MSG_SC_Mail_InfoEx& mailInfo)
{
	acl::query query;
	query.create_sql("select * from mail where mailid = :mailid limit 1")
		.set_format("mailid", "%u", dwMailID);
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query user game mail fail: " << query.to_string().c_str();
		return false;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		mailInfo.dwMailID = (CT_DWORD)atoi((*row)["mailid"]);
		mailInfo.dwUserID = (CT_DWORD)atoi((*row)["userid"]);
		std::string strTitle = (*row)["title"];
		std::string strContent = (*row)["content"];
		_snprintf_info(mailInfo.szTitle, sizeof(mailInfo.szTitle), "%s", Utility::Ansi2Utf8(strTitle.c_str()).c_str());
		_snprintf_info(mailInfo.szContent, sizeof(mailInfo.szContent), "%s", Utility::Ansi2Utf8(strContent.c_str()).c_str());
		//mailInfo.wGem = (CT_WORD)atoi((*row)["gem"]);
		mailInfo.dScore = atoll((*row)["score"])*TO_DOUBLE;
		mailInfo.cbScoreType = (CT_BYTE)atoi((*row)["scoreType"]);
		mailInfo.cbMailType = (CT_BYTE)atoi((*row)["mailType"]);
		mailInfo.cbState = (CT_BYTE)atoi((*row)["state"]);
		_snprintf_info(mailInfo.szTime, sizeof(mailInfo.szTime), "%s", Utility::Ansi2Utf8((*row)["sendTime"]).c_str());
	}

	db->free_result();

	return true;
}

CT_VOID CLoginServerThread::SendMail(tagUserMail& mail)
{
	//更新db
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("INSERT INTO mail(userid, title, content, gem, score, scoreType, mailType, adminid, state, sendTime, expireTime) VALUES\
						(:userid, :title, :content, 0, :score, :scoreType, :mailType, 0, :state, :sendTime, :expireTime)")
			.set_format("userid", "%u", mail.dwUserID)
			.set_format("title", "%s", mail.szTitle)
			.set_format("content", "%s", mail.szContent)
			.set_format("score", "%lld", mail.llScore)
			.set_format("scoreType", "%d", mail.cbScoreChangeType)
			.set_format("mailType", "%d", mail.cbMailType)
			.set_format("state", "%d", mail.cbState)
			.set_date("sendTime", mail.dwSendTime)
			.set_date("expireTime", mail.dwExpiresTime);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert mail fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}
	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryScoreInfo(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	QueryScoreInfo(db, dwUserID);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryScoreInfo(acl::db_handle* db, CT_DWORD dwUserID)
{
	acl::query query;
	query.create_sql("select revenue, winCount, lostCount, drawCount, fleeCount, playTime from gameScoreData \
		 where userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query user score info fail: " << query.to_string().c_str();
		return;
	}

	if (db->length() == 0)
	{
		db->free_result();
		query.reset();
		query.create_sql("insert into gameScoreData(userid, revenue, winCount, lostCount, drawCount, fleeCount, playTime) VALUES(:userid , 0, 0, 0, 0, 0, 0)")
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user score info fail: " << query.to_string().c_str();
			return;
		}
	}
	else
	{
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			m_scoreData.dwRevenue = (CT_DWORD)atoi((*row)["revenue"]);
			m_scoreData.dwWinCount = (CT_DWORD)atoi((*row)["winCount"]);
			m_scoreData.dwLostCount = (CT_DWORD)atoi((*row)["lostCount"]);
			m_scoreData.dwDrawCount = (CT_DWORD)atoi((*row)["drawCount"]);
			m_scoreData.dwFleeCount = (CT_DWORD)atoi((*row)["fleeCount"]);
			m_scoreData.dwPlayTime = (CT_DWORD)atoi((*row)["playTime"]);
		}
	}
	db->free_result();
}

CT_VOID CLoginServerThread::QueryTaskInfo(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	QueryTaskInfo(db, dwUserID);

	pool->put(db);
}

CT_VOID	CLoginServerThread::QueryTaskInfo(acl::db_handle* db, CT_DWORD dwUserID)
{
	acl::query query;
	query.create_sql("SELECT * FROM userTask WHERE userid = :userid")
		.set_format("userid", "%u", dwUserID);
	
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query user task info fail, errno: " << db->get_errno() << ", error msg: " << db->get_error();
		return;
	}

	tagUserTask userTask(0);
	for (size_t i = 0; i < db->length(); i++)
	{
		userTask.Reset();
		const acl::db_row* row = (*db)[i];
		userTask.wTaskID = (CT_WORD)atoi((*row)["taskid"]);
		userTask.dwCurrProgress = (CT_DWORD)atoi((*row)["currProgress"]);
		userTask.dwReceiveTime= (CT_DWORD)atoi((*row)["recvTime"]);
		userTask.bFinish = atoi((*row)["finish"]) > 0 ? true : false;
		m_vecTask.push_back(userTask);
	}

	db->free_result();
}

CT_VOID CLoginServerThread::QueryGoodCardInfo(UserAccountsBaseData& userAccountsBaseData)
{
    /*acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game db handle fail.";
        return;
    }

    do
    {
    	acl::query query;
    	query.create_sql("SELECT zjhCount, zjhParam, qznnCount, qznnParam, goodCardAdjustTime from game.gamescoredata where userid = :userid LIMIT 1")
    	.set_format("userid", "%u", userAccountsBaseData.dwUserID);

    	if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "query user good card info fail, errno: " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			userAccountsBaseData.dwZjhCount = (CT_DWORD)atoi((*row)["zjhCount"]);
			userAccountsBaseData.cbZjhGoodCardParam = (CT_BYTE)atoi((*row)["zjhParam"]);
			userAccountsBaseData.dwQznnCount = (CT_DWORD)atoi((*row)["qznnCount"]);
			userAccountsBaseData.cbQznnGoodCardParam = (CT_BYTE)atoi((*row)["qznnParam"]);
			userAccountsBaseData.dwGoodCardTime = (CT_DWORD)atoi((*row)["goodCardAdjustTime"]);
		}
    }while (0);

    db->free_result();
    pool->put(db);*/
}

CT_VOID CLoginServerThread::UpdateUserGoodCardInfo(UserAccountsBaseData& userAccountsBaseData)
{
	/*acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("update gameScoreData set zjhCount = :zjhCount, zjhParam = :zjhParam, qznnCount = :qznnCount, qznnParam= :qznnParam, goodCardAdjustTime = :goodCardAdjustTime  where userid = :userid")
			.set_format("zjhCount", "%u", userAccountsBaseData.dwZjhCount)
			.set_format("zjhParam", "%d", userAccountsBaseData.cbZjhGoodCardParam)
			.set_format("qznnCount", "%u", userAccountsBaseData.dwQznnCount)
			.set_format("qznnParam", "%d", userAccountsBaseData.cbQznnGoodCardParam)
			.set_format("goodCardAdjustTime", "%u", userAccountsBaseData.dwGoodCardTime)
			.set_format("userid", "%u", userAccountsBaseData.dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update good card info fail, user id: " << userAccountsBaseData.dwUserID << ", errno: " << db->get_errno()
			<< ", error: " << db->get_error()<< ", sql: " << query.to_string().c_str();
			break;
		}
	}while (0);

	db->free_result();
	pool->put(db);*/
}

CT_VOID CLoginServerThread::QueryUserEarnScore(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	CT_LONGLONG llTodayEarnScore = 0;
	do
	{
		acl::query query;
		query.create_sql("SELECT earnScore from todayEarnScore WHERE userid = :userid  LIMIT 1")
			.set_format("userid", "%u", dwUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select earn score fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			llTodayEarnScore = atoll((*row)["earnScore"]);
		}

		db->free_result();

	} while (0);

	pool->put(db);

	SetUserEarnScore(dwUserID, llTodayEarnScore);
}

CT_VOID CLoginServerThread::QueryUserTodayRankInfo(CT_DWORD dwUserID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	CT_DWORD dwOnlineTime = 0;
	CT_WORD  wTaskFinishCount = 0;
	CT_DWORD dwTaskRewardScore = 0;
	do
	{
		acl::query query;
		query.create_sql("SELECT online from today_online_time WHERE userid = :userid  LIMIT 1")
		.set_format("userid", "%u", dwUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select today_online_time fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwOnlineTime = atoi((*row)["online"]);
		}

		db->free_result();

		query.reset();
		query.create_sql("SELECT finishCount, rewardScore from today_task_count WHERE userid = :userid  LIMIT 1")
		.set_format("userid", "%u", dwUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select today_task_count fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			wTaskFinishCount = atoi((*row)["finishCount"]);
			dwTaskRewardScore = atoi((*row)["rewardScore"]);
		}

		db->free_result();
	} while (0);

	pool->put(db);

	SetUserTodayRankInfo(dwUserID, dwOnlineTime, wTaskFinishCount, dwTaskRewardScore);
}

CT_BYTE CLoginServerThread::BindUserPromoterID(acl::db_handle* db, CT_DWORD dwUserID, CT_DWORD dwPromoterID)
{
	//检查代理ID是否正确
	acl::query query;
	query.create_sql("select userid from promotion.promoter where userid = :promoterid  LIMIT 1")
		.set_format("promoterid", "%u", dwPromoterID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select promoter fail: " << query.to_string().c_str();
		return 2; 
	}
	if (db->length() == 0)
	{
		return 2;
	}

	db->free_result();

	query.reset();
	query.create_sql("update userinfo set promoter = :promoterid where userid = :userid LIMIT 1")
		.set_format("promoterid", "%u", dwPromoterID)
		.set_format("userid", "%u",	dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update user bind promoter fail: " << query.to_string().c_str();
		return 2;
	}
	
	db->free_result();

	return 0;
}

CT_VOID CLoginServerThread::UpdateUserBankPwd(CT_DWORD dwUserID, CT_CHAR* pBankPwd)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("UPDATE `user` SET bankpass = :bankpass WHERE userid = :userid  LIMIT 1")
			.set_format("bankpass", "%s", pBankPwd)
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user bank password fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		db->free_result();

	} while (0);

	pool->put(db);
}

CT_BOOL CLoginServerThread::CheckAliPayOrBankCardLimited(const CT_CHAR* pPayAccount)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return false;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return false;
	}
	CT_BOOL bForbid = false;
	do 
	{
		acl::query query;
		query.create_sql("SELECT body FROM limits WHERE body = :account LIMIT 1")
			.set_format("account", "%s", pPayAccount);


		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select alipay bank card  fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		if (db->length() != 0)
		{
			bForbid = true;
		}
		
	
	} while (0);

	db->free_result();
	pool->put(db);

	return bForbid;
}

CT_VOID CLoginServerThread::InsertExchangeOrder(MSG_CS_Exchange_Score_To_Rmb* pExchange)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

    CT_DOUBLE dFee = EXCHANGE_REVENUE;
    for (auto& it : m_vecExchangeInfo)
    {
        if (it.cbChannelID == pExchange->cbType)
        {
            dFee = ceil(it.wHandlingFee) / 1000;
            break;
        }
    }

    CT_LONGLONG llExchangeScore = pExchange->dwExchangeScore * TO_LL;
    CT_DOUBLE dRevenue = llExchangeScore*dFee;
	CT_DWORD dwReviseRevenue = (CT_DWORD)round(dRevenue);
	do
	{
		acl::query query;
		if (pExchange->cbType == 2)
		{
			query.create_sql("INSERT INTO treasure.exchange(userid, fixdate, price, pay, applydate, reason, status,type, aliAccount,aliName,bankAccount,bankCard,bankAddress,bankType) \
			(SELECT :userid, :fixdate, :price, :pay, 0, '', 8, :type, aliAccount, aliName, '', '', '', 0 from usercard WHERE userid = :userid)")
				.set_format("userid", "%u", pExchange->dwUserID)
				.set_format("fixdate", "%u", (CT_DWORD)time(NULL))
				.set_format("price", "%lld", llExchangeScore)
				.set_format("pay", "%lld", llExchangeScore - dwReviseRevenue)
				.set_format("type", "%u", pExchange->cbType);
		}
		else
		{
			query.create_sql("INSERT INTO treasure.exchange(userid, fixdate, price, pay, applydate, reason, status,type, aliAccount,aliName,bankAccount,bankCard,bankAddress,bankType) \
			(SELECT :userid, :fixdate, :price, :pay, 0, '', 8, :type, '', '', bankAccount, bankCard, bankAddress, bankType from usercard WHERE userid = :userid)")
				.set_format("userid", "%u", pExchange->dwUserID)
				.set_format("fixdate", "%u", (CT_DWORD)time(NULL))
				.set_format("price", "%lld", llExchangeScore)
				.set_format("pay", "%lld", llExchangeScore - dwReviseRevenue)
				.set_format("type", "%u", pExchange->cbType);
		}

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert into exchange fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

        /*if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
        {
            int nError = db->get_errno();
            LOG(WARNING) << "insert report info fail, errno = " << nError << ", error: " << db->get_error();
            break;
        }
        int nExchangeId = 0;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            nExchangeId = atoi((*row)["LAST_INSERT_ID()"]);
        }
        db->free_result();

        std::shared_ptr<tagExchangePost> exchangePostPtr = m_ExchangePostThreadPtr->GetFreeExchangePostQue();
        exchangePostPtr->nExchangeId = nExchangeId;
        exchangePostPtr->nPlatformId = GetUserPlatformId(pExchange->dwUserID);
        m_ExchangePostThreadPtr->InsertExchangePostQue(exchangePostPtr);*/

	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::QueryExchangeOrderRecord(CT_DWORD dwUserID, std::vector<MSG_SC_Exchange_Rmb_Record>& vecRecord)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM treasure.exchange WHERE userid = :userid order by id desc LIMIT 10")
			.set_format("userid", "%u", dwUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "query exchange record fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		MSG_SC_Exchange_Rmb_Record record;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			record.dwExchangeScore = (CT_DWORD)(atoi((*row)["price"])*TO_DOUBLE);
			record.cbState = (CT_BYTE)atoi((*row)["status"]);
			CT_DWORD dwTime = atoi((*row)["fixdate"]);
			_snprintf_info(record.szTime, sizeof(record.szTime), "%s", Utility::ChangeTimeToString(dwTime).c_str());
			int nType = atoi((*row)["type"]);
			if (nType == 2)
			{
				std::string account = (*row)["aliAccount"];
				_snprintf_info(record.szAccount, sizeof(record.szAccount), "%s", account.c_str());
			}
			else
			{
				std::string account = (*row)["bankCard"];
				_snprintf_info(record.szAccount, sizeof(record.szAccount), "%s", account.c_str());
			}
			vecRecord.push_back(record);
		}
		db->free_result();

	} while (0);

	pool->put(db);
}

CT_WORD CLoginServerThread::GetTodayExchangeOrderCount(CT_DWORD dwUserID, CT_BYTE cbExchangeChannelID)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return 0;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return 0;
	}
	
	CT_WORD wCount = 0;
	do
	{
		acl::query query;
		std::string strToday = Utility::GetTimeNowString("%Y-%m-%d");
		query.create_sql("SELECT count(id) from treasure.exchange WHERE userid = :userid AND type = :type AND (`status` = 6 OR `status` = 8 OR `status` = 10 OR `status` = 18) AND FROM_UNIXTIME(fixdate) > :today ")
		.set_format("userid", "%u", dwUserID)
		.set_format("type", "%d" , cbExchangeChannelID)
		.set_format("today", "%s", strToday.c_str());
		
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "GetTodayExchangeOrderCount fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			wCount = (CT_DWORD)(atoi)((*row)["count(id)"]);
		}
		db->free_result();
		
	} while (0);
	
	pool->put(db);
	
	return wCount;
}

CT_VOID CLoginServerThread::QuyerRechargeOrderRecord(CT_DWORD dwUserID, std::vector<MSG_SC_RechargeRecord>& vecRecord)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		//query.create_sql("SELECT * FROM treasure.orders WHERE userid = :userid ORDER BY id DESC LIMIT 20")
		//	.set_format("userid", "%u", dwUserID);
        query.create_sql("SELECT * FROM  (SELECT  a.`pay`*100 as `pay`, a.`type`, a.`status`, a.`fixdate`, a.`orderid` FROM treasure.orders a WHERE userid = :userid ORDER BY id DESC LIMIT 20) t1\n"
                         "UNION SELECT * FROM (SELECT b.score as `pay`, 5, 2, b.`fixdate`, b.`id` as `orderid` FROM promotion.`rewards` b where receiver = :userid ORDER BY id DESC LIMIT 20) t2 "
                         "ORDER BY fixdate desc LIMIT 20")
                         .set_format("userid", "%u", dwUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "query excharge record fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		MSG_SC_RechargeRecord record;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			record.dPay = atoi((*row)["pay"])*TO_DOUBLE;
			record.cbType = (CT_BYTE)atoi((*row)["type"]);
			record.cbStatus =  (CT_BYTE)atoi((*row)["status"]);
			record.dwTime = (CT_DWORD)(atoi((*row)["fixdate"]));
			_snprintf_info(record.szOrderId, sizeof(record.szOrderId), "%s", (*row)["orderid"]);
			vecRecord.push_back(record);
		}
		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::InsertModifyRecord(CT_DWORD dwUserID, CT_BYTE cbType, CT_CHAR* pContent)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	
	do
	{
		acl::query query;
		query.create_sql("INSERT INTO record_edit_info(userid, fixdate, type, content) VALUES (:userid, :fixdate, :type, :content)")
			.set_format("userid", "%u", dwUserID)
			.set_format("fixdate", "%u", (CT_DWORD)time(NULL))
			.set_format("type", "%d", cbType)
			.set_format("content", "%s", pContent);


		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert into record_edit_info fail. sql:" << query.to_string().c_str() << ", errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::SendKickUserMsg(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, CT_DWORD dwUserID, CT_BOOL bSendToClient)
{
	MSG_LS2P_KickUser kickUser;
	kickUser.dwUserID = dwUserID;
	// kickUser.wGameID = wGameID;
	kickUser.dwNetAddr = uClientValue;
	kickUser.bSendToClient = bSendToClient;

	CNetModule::getSingleton().Send(pSocket, MSG_PLS_MAIN, SUB_LS2P_KICK_USER, &kickUser, sizeof(kickUser));
}

CT_VOID CLoginServerThread::SendProxyServerLoginSucc(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, UserAccountsBaseData& accountData, CT_WORD wGameID)
{
	MSG_LS2P_UserLoginSucc userLoginSucc;
	memset(&userLoginSucc, 0, sizeof(userLoginSucc));
	userLoginSucc.dwUserID = accountData.dwUserID;
	//userLoginSucc.dwGem = accountData.dwGem;
	userLoginSucc.llScore = accountData.llScore;
	userLoginSucc.llBankScore = accountData.llBankScore;
	//userLoginSucc.wGameID = wGameID;
	userLoginSucc.cbPlatformId = accountData.cbPlatformID;
	userLoginSucc.cbSex = accountData.cbGender;
	userLoginSucc.cbVipLevel = accountData.cbVipLevel;
	userLoginSucc.cbVipLevel2 = accountData.cbVipLevel2;
	userLoginSucc.dwRecharge = accountData.dwTotalRecharge;
	userLoginSucc.cbHeadID = accountData.cbHeadId;
	userLoginSucc.dwChannelID = accountData.dwChannelID;
	userLoginSucc.dwClientChannelID = accountData.dwClientChannelID;
	userLoginSucc.cbShowExchange = accountData.cbShowExchange;
	//_SET_BIT32(userLoginSucc.cbPlayMode, PRIMARY_MODE, m_cbPrivateMode == 0 ? false : true);
	//_SET_BIT32(userLoginSucc.cbPlayMode, SCORE_MODE, m_cbScoreMode == 0 ? false : true);
    std::string strMachineType(accountData.szMachineType);
	//if (strcmp(accountData.szMachineType, "iPhone") == 0 || strcmp(accountData.szMachineType, "iPad") == 0)
	if (strMachineType.find("iPhone") != std::string::npos
	    || strMachineType.find("iPad") != std::string::npos)
	{
		userLoginSucc.cbMachineType = MACHINE_TYPE_IOS;
	}
	else
	{
		userLoginSucc.cbMachineType = MACHINE_TYPE_ANDROID;
	}

	std::string strProvince(accountData.szProvince);
	userLoginSucc.cbProvinceCode = GetProviceId(strProvince);
	/*std::size_t found = strProvince.find_first_of("新疆");
    if (found != std::string::npos)
    {
        userLoginSucc.cbProvinceCode = en_XINJIANG;
    }
    else
    {
        userLoginSucc.cbProvinceCode = en_OTHER;
    }*/

	userLoginSucc.dwNetAddr = uClientValue;
	_snprintf_info(userLoginSucc.szNickName, sizeof(userLoginSucc.szNickName), "%s", accountData.szNickName);
	//_snprintf_info(userLoginSucc.szHeadUrl, sizeof(userLoginSucc.szHeadUrl), "%s", accountData.szHeadUrl);
	 
	CNetModule::getSingleton().Send(pSocket, MSG_PLS_MAIN, SUB_LS2P_USER_LOGIN_SUCC, &userLoginSucc, sizeof(userLoginSucc));
}

CT_BYTE CLoginServerThread::GoDBAccountLoginOrRegister(acl::db_handle* db, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData)
{
	if (db == NULL)
	{
		return LOGIN_NETBREAK;
	}

	//判断帐号是否为空
	if (strlen(pLoginData->szAccount) == 0)
	{
		LOG(WARNING) << "account is empty? ";
		return LOGIN_REGISTER_FAIL;
	}

	//检验协议是否正确
	acl::string strVerifyCode(pLoginData->szMachineSerial);
	strVerifyCode += pLoginData->szAccount;
	m_MD5.reset();
	m_MD5.update(strVerifyCode.c_str(), strVerifyCode.length());
	m_MD5.finish();
	const char* pMd5VerifyCode = m_MD5.get_string();
	if (strcmp(pMd5VerifyCode, pLoginData->szPassword) != 0)
	{
		LOG(WARNING) << "account verify fail. cal md5: " << pMd5VerifyCode << ", user md5: " << pLoginData->szPassword;
		return LOGIN_REGISTER_FAIL;
	}
	
	acl::query query;
	if (pLoginData->dwUserID != 0)
	{
		query.create_sql("SELECT userid, platformId, username, mobileNum, status FROM `user` where userid = :userid LIMIT 1")
			.set_format("userid", "%u", pLoginData->dwUserID);
	}
	else
	{
		query.create_sql("SELECT userid, platformId, username, mobileNum, status FROM `user` where platformId = :platformId AND username = :username LIMIT 1")
			.set_format("platformId", "%d", pLoginData->cbPlatformID)
			.set_format("username", "%s", pLoginData->szAccount);
	}


	CT_DWORD dwUserID = 0;
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select account info failed, user name : " << pLoginData->szAccount << ", errno: " << db->get_errno() << ", error: " << db->get_error();
		return LOGIN_NETBREAK;
	}

	//如果没有的话，就需要注册
	if (db->length() == 0)
	{
		db->free_result();

		unsigned int nIpNum = Utility::ip2Int(pLoginData->szClientIP);
		//检查是否超过限制
		CT_WORD dwRegisterCount = GetIpRegisterCount(nIpNum);
		if (dwRegisterCount >= m_wIpRegisterLimite)
		{
			return LOGIN_REGISTER_FAIL;
		}

		CT_DWORD dwChannelID = pLoginData->dwChannelID;
		//如果是iPhone注册则检测是否有IP匹配
		//if (strcmp(pLoginData->szMachineType, "iPhone") == 0)
		if (pLoginData->cbIsGetChannelID == 0)
		{
			//改由redis操作
			/*
			query.reset();
		
			query.create("SELECT * from channel_ip where ip = :ipNum ORDER BY fixdate DESC LIMIT 1")
				.set_format("ipNum", "%u", nIpNum);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select channel ip info failed " << ", errno:" << db->get_errno() << ", error: " << db->get_error();
				return LOGIN_NETBREAK;
			}

			//渠道ID
			CT_BOOL bFindChannel = false;
			CT_DWORD dwRecordTime = 0;
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				dwChannelID = atoi((*row)["chid"]);
                dwRecordTime = (CT_DWORD)atoll((*row)["fixdate"]);
				bFindChannel = true;
			}
			db->free_result();

			//删除这条IP记录
			if (bFindChannel)
			{
				query.reset();
				query.create("DELETE FROM channel_ip where ip = :ipNum AND chid = :chid AND fixdate = :fixdate LIMIT 1")
					.set_format("ipNum", "%u", nIpNum)
					.set_format("chid", "%d", dwChannelID)
					.set_format("fixdate", "%u", dwRecordTime);

				if (db->exec_update(query) == false)
				{
					LOG(WARNING) << "delete channel ip info failed " << ", errno:" << db->get_errno() << ", error: " << db->get_error();
				}
			}
			 */

			m_redis2.clear();
			acl::string strChannelID;
			if (m_redis2.lpop(pLoginData->szClientIP, strChannelID) > 0)
			{
				int nChannelID = atoi(strChannelID.c_str());
				if (nChannelID != 0)
					dwChannelID = nChannelID;
				//LOG(WARNING) << "channel id: " << dwChannelID;
			}
		}

		//首先从帐号备库里面 取出帐号ID
		dwUserID = GetNewUserIDFromDB();
		if (dwUserID == 0)
		{
			return LOGIN_REGISTER_FAIL;
		}
		/*query.reset();
		query.create_sql("SELECT userid from random_userid LIMIT :randValue, 1 FOR UPDATE")
			.set_parameter("randValue", rand() % MAX_RAND_USERID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select user id fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			LOG(WARNING) << "sql : " << query.to_string().c_str();
			db->rollback();
			return REGISTER_UNKNOWN;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwUserID = atoi((*row)["userid"]);
		}
		if (dwUserID == 0)
		{
			LOG(WARNING) << "USER ID: " << dwUserID;
		}
		db->free_result();*/

		db->free_result();
		query.reset();
		query.create_sql("INSERT INTO `user`(userid, promoterid, platformId, username, loginpass, bankpass, salt, nickname, status)"
                   " VALUES(:userid, :promoterid, :platformId, :username, '', '', '', '', 1)")
            .set_format("userid", "%d", dwUserID)
            .set_format("promoterid", "%u", dwChannelID)
			.set_format("platformId", "%d", pLoginData->cbPlatformID)
			.set_format("username", "%s", pLoginData->szAccount);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user fail, user name: " << pLoginData->szAccount << ", error = " << db->get_errno() << ", error msg: " << db->get_error();
			return LOGIN_REGISTER_FAIL;
		}

		/*if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
		{
			LOG(WARNING) << "select last insert id fail, error = " << db->get_errno();
			//db->rollback();
			return LOGIN_REGISTER_FAIL;
		}
	
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwUserID = atoi((*row)["LAST_INSERT_ID()"]);
		}
		db->free_result();*/

		//更新玩家用户昵称
		acl::string strNickName;
		//strNickName.format("游客%u", dwUserID);
		strNickName.format("%u", dwUserID);
		query.reset();
		query.create_sql("UPDATE `user` SET nickname = :nickname WHERE userid = :userid")
			.set_format("nickname", "%s", strNickName.c_str())
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user info fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
			return LOGIN_REGISTER_FAIL;
		}

		//检测是否开启兑换
		accountData.cbShowExchange = GetChannelShowExchange(dwChannelID);

		//注册赠送金币
		CT_DWORD dwRegisterPresentScore = GetChannelPresentScore(dwChannelID, en_Register);

		//随机一个性别
		int nRandNum = rand();
		int nRandSex = nRandNum % 2 + 1;

		int nRandHeadId = nRandNum % en_HeadId_Count + en_HeadId_Boy_Start;
		query.reset();
		query.create_sql("INSERT INTO userinfo(userid, gender, headId, mobile, gem, score,bankscore, manager, promoter, regdate, regip, \
			regmachineType, regserial, lastdate, lastip, lastmachineType, lastserial, logincount, showExchange) \
			VALUES(:userid, :gender,:headId, 0, :gem, :score,:bankscore, 0, :promoterid, UNIX_TIMESTAMP(NOW()), :clientip, \
			:machinetype ,:serial, UNIX_TIMESTAMP(NOW()), :clientip, :machinetype , :serial, 1, :showExchange)")
			.set_format("userid", "%u", dwUserID)
			.set_format("gender", "%d", nRandSex)
			//.set_format("headurl", "%s", "")
			.set_format("headId", "%d", nRandHeadId)
			.set_format("gem", "%d", m_iRegisterPresentGem)
			.set_format("score", "%d", dwRegisterPresentScore)
			.set_format("bankscore", "%u", 0)
			.set_format("promoterid", "%u", dwChannelID)
			.set_format("clientip", "%s", pLoginData->szClientIP)
			.set_format("machinetype", "%s", pLoginData->szMachineType)
			.set_format("serial", "%s", pLoginData->szMachineSerial)
			.set_format("showExchange", "%d", accountData.cbShowExchange);
			
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user info fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno() << ", error: " << db->get_error();
			return LOGIN_REGISTER_FAIL;
		}

		query.reset();
		query.create_sql("INSERT INTO userdata(userid, vip, pay) VALUES(:userid,0,0)")
			.set_format("userid", "%u", dwUserID);
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user info fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
			return LOGIN_REGISTER_FAIL;
		}

		//query.reset();
		//query.create_sql("DELETE FROM random_userid WHERE userid = :userid")
		//	.set_format("userid", "%u", dwUserID);

		//if (db->exec_update(query) == false)
		//{
		//	LOG(WARNING) << "delete user id fail. account: " << pLoginData->szAccount << ", errno: " << db->get_errno() << ", error: " << db->get_errno();
		//	return LOGIN_REGISTER_FAIL;
		//}

		if (accountData.cbShowExchange)
		{
			accountData.cbNewAccount = 1;
		}
		else
		{
			accountData.cbNewAccount = 2;
		}
	
		//记录赠送金币
		InsertScoreChangeRecord(dwUserID, 0, 0, 0, dwRegisterPresentScore, PRESENTED_SCORE);

		//苹果审核帐号发一封邮件
		if (m_cbOpenApplePay == 1)
		{
			tagUserMail userMail;
			userMail.cbMailType = 2;
			userMail.cbState = 0;
			userMail.dwSendTime = Utility::GetTime();
			userMail.dwExpiresTime = userMail.dwSendTime + 86400 * 365; //默认一年的有效期
			userMail.dwUserID = dwUserID;
			userMail.llScore = 0;
			_snprintf_info(userMail.szTitle, sizeof(userMail.szTitle), "%s", "欢迎来到星空百人牛牛");
			_snprintf_info(userMail.szContent, sizeof(userMail.szContent), "%s", "欢迎来到星空百人牛牛，新人登陆可以获得新手礼包, 祝你玩得开心！");
			SendMail(userMail);
		}

		CT_BOOL bSendRegisterMail = IsChannelSendRegisterMail(dwChannelID);
		if (bSendRegisterMail)
		{
			CT_DWORD dwShowExchangeRecharge = GetChannelShowExchangeRecharge(dwChannelID);
			tagUserMail userMail;
			userMail.cbMailType = 2;
			userMail.cbState = 0;
			userMail.dwSendTime = Utility::GetTime();
			userMail.dwExpiresTime = userMail.dwSendTime + 86400 * 365; //默认一年的有效期
			userMail.dwUserID = dwUserID;
			userMail.llScore = 0;
			_snprintf_info(userMail.szTitle, sizeof(userMail.szTitle), "%s", "兑换开启通知");
			_snprintf_info(userMail.szContent, sizeof(userMail.szContent), "亲爱的玩家：\n        由于政策因素您现在无法使用兑换功能，当充值达到%u元，即可自动开启兑换功能。如有疑问请联系我们24小时客服。感谢您对本游戏的支持！祝您好运连连！", dwShowExchangeRecharge);
			SendMail(userMail);
		}

		IncreaseRegisterCount(nIpNum);
	}
	else
	{
		int nStatus = 1;
		acl::string strMobileNum;
		acl::string strUserName;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwUserID = atoi((*row)["userid"]);
			accountData.cbPlatformID = (CT_BYTE)atoi((*row)["platformId"]);
			strUserName = (*row)["username"];
			strMobileNum = (*row)["mobileNum"];
			nStatus = atoi((*row)["status"]);
		}
		db->free_result();

		if (nStatus == en_Account_SEAL)
		{
			return LOGIN_SEAL_ACCOUNTS;
		}

		if (strUserName != pLoginData->szAccount)
        {
			//LOG(WARNING) << "user name not equ!";
            return LOGIN_ACCOUNTS_NOT_EXIST;
        }

		if (strMobileNum.compare("0") != 0)
		{
			_snprintf_info(accountData.szMobileNum, sizeof(accountData.szMobileNum), "%s", strMobileNum.c_str());
			return LOGIN_PASSWORD_ERCOR;
		}

		query.reset();
		query.create_sql("UPDATE userinfo SET lastdate = UNIX_TIMESTAMP(NOW()), lastip = :ip,\
			 lastmachineType = :machineType, lastserial = :serial, logincount = logincount+1 WHERE userid = :userid")
			.set_format("ip", "%s", pLoginData->szClientIP)
			.set_format("machineType", "%s", pLoginData->szMachineType)
			.set_format("serial", "%s", pLoginData->szMachineSerial)
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
			return LOGIN_NETBREAK;
		}
		db->free_result();
	}

	//重新读取玩家数据
	query.reset();
	query.create_sql("SELECT `user`.nickname,`user`.bankpass, userinfo.gender, userinfo.score, userinfo.bankscore, userinfo.headId, userinfo.promoter, userinfo.showExchange, \
		userdata.vip, userdata.vip2, userdata.pay from userinfo \
		INNER JOIN `user` ON `user`.userid = userinfo.userid \
		INNER JOIN userdata ON userdata.userid = userinfo.userid WHERE userinfo.userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select user info fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
		return LOGIN_NETBREAK;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		accountData.cbGender = atoi((*row)["gender"]);
		accountData.llScore = atoll((*row)["score"]);
		accountData.llBankScore = atoll((*row)["bankscore"]);
		std::string strNickName = ((*row)["nickname"]);
		//std::string strHeadUrl = ((*row)["headimage"]);
		std::string strBankpass = ((*row)["bankpass"]);
		accountData.cbHeadId = atoi((*row)["headId"]);
		accountData.dwChannelID = atoi((*row)["promoter"]);
		accountData.cbVipLevel = (CT_BYTE)atoi((*row)["vip"]);
		accountData.cbVipLevel2 = (CT_BYTE)atoi((*row)["vip2"]);
		accountData.dwTotalRecharge = (CT_DWORD)atoi((*row)["pay"]);
		accountData.cbShowExchange = (CT_BYTE)atoi((*row)["showExchange"]);
		_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
		//_snprintf_info(accountData.szHeadUrl, sizeof(accountData.szHeadUrl), "%s", strHeadUrl.c_str());
		_snprintf_info(accountData.szBankPassword, sizeof(accountData.szBankPassword), "%s", strBankpass.c_str());
	}
	db->free_result();

	//读取玩家银行卡信息
	query.reset();
	query.create_sql("SELECT * FROM usercard WHERE userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select user card fail, user id: " << dwUserID << ", error: " << db->get_errno() << ", error msg: " << db->get_error();
		return LOGIN_NETBREAK;
	}
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		std::string strAliAccount = ((*row)["aliAccount"]);
		std::string strAliName = ((*row)["aliName"]);
		std::string strBankCard = ((*row)["bankCard"]);
		std::string strBankName = ((*row)["bankAccount"]);
		_snprintf_info(accountData.szAliPayAccout, sizeof(accountData.szAliPayAccout), "%s", Utility::Ansi2Utf8(strAliAccount).c_str());
		_snprintf_info(accountData.szAliPayName, sizeof(accountData.szAliPayName), "%s", Utility::Ansi2Utf8(strAliName).c_str());
		_snprintf_info(accountData.szBankCardNum, sizeof(accountData.szBankCardNum), "%s", Utility::Ansi2Utf8(strBankCard).c_str());
		_snprintf_info(accountData.szBankCardName, sizeof(accountData.szBankCardName), "%s", Utility::Ansi2Utf8(strBankName).c_str());
	}
	db->free_result();
	
	accountData.dwUserID = dwUserID;
	return 0;
}

/*CT_BYTE CLoginServerThread::GoDBAccountLoginOrRegister(MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData)
{
	CT_DWORD dwSuc = -1;
	m_accountDBCon.Reset();
	m_accountDBCon.SetPcName("G_OUT_UserLoginAccountsLoginOrReg");
	m_accountDBCon.AddParm("in_PlatformID", pLoginData->cbPlatformID);
	m_accountDBCon.AddParm("in_Account", pLoginData->chPlatformAccount);
	m_accountDBCon.AddParm("in_Gender", pLoginData->cbGender);
	m_accountDBCon.AddParm("in_NickName", pLoginData->chNickName);
	m_accountDBCon.AddParm("in_PWD", pLoginData->chPlatformPWD);
	m_accountDBCon.AddParm("in_HeadUrl", pLoginData->chHeadUrl);
	m_accountDBCon.AddParm("in_Serial", pLoginData->chMachineSerial);
	m_accountDBCon.AddParm("in_MachineType", pLoginData->chMachineType);
	m_accountDBCon.AddParm("in_ClientIP", pLoginData->chClientIP);

	if (m_accountDBCon.Exec())
	{
		CCDBResult* pResult = m_accountDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			while (pResult->MoveNextRow())
			{
				pResult->GetValue(0, dwSuc);
				if (0 == dwSuc)
				{
					pResult->GetValue(1, accountData.dwUserID);
					pResult->GetValue(2, accountData.dwGem);
					pResult->GetValue(3, accountData.cbGender);
					std::string strAnsiNickName; 
					pResult->GetValue(4, strAnsiNickName);
					std::string strUtf8 = Utility::Ansi2Utf8(strAnsiNickName);
					_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", strUtf8.c_str());
				}
				else
				{
					dwSuc = LOGIN_REGISTER_FAIL;
				}
				break;
			}
			pResult->Release();
			if (0 != dwSuc)
			{
				return dwSuc;
			}
		}
	}
	else
	{
		//帐户DB错误
		OnAccountDBError();
		return LOGIN_NETBREAK;
	}

	return dwSuc;
}*/

CT_BYTE CLoginServerThread::GoDBAccountRegister(acl::db_handle* db, MSG_CS_LoginDataEx* pRegister, UserAccountsBaseData& accountData)
{
	if (db == NULL)
	{
		return REGISTER_UNKNOWN;
	}

	acl::query query;
	query.create_sql("SELECT * FROM `user` where mobileNum = :username LIMIT 1")
		.set_format("username", "%s", pRegister->szAccount);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "check account exist failed, user name : " << pRegister->szAccount << ", error: " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	if (db->length() != 0)
	{
		db->free_result();

		return REGISTER_ACCOUNT_REPEAT;
	}
	db->free_result();

	CT_DWORD dwChannelID = pRegister->dwChannelID;
	//如果是iPhone注册则检测是否有IP匹配
	if (strcmp(pRegister->szMachineType, "iPhone") == 0)
	{
		query.reset();
		unsigned int nIpNum = Utility::ip2Int(pRegister->szClientIP);
		query.create("SELECT * from channel_ip where ip = :ipNum LIMIT 1")
			.set_format("ipNum", "%u", nIpNum);

		LOG(WARNING) << query.to_string().c_str();
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select channel ip info failed " << ", errno:" << db->get_errno() << ", error: " << db->get_error();
			return LOGIN_NETBREAK;
		}

		//渠道ID
		CT_BOOL bFindChannel = false;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwChannelID = atoi((*row)["chid"]);
			bFindChannel = true;
		}
		db->free_result();

		//删除这条IP记录
		if (bFindChannel)
		{
			query.reset();
			query.create("DELETE FROM channel_ip where ip = :ipNum LIMIT 1")
				.set_format("ipNum", "%u", nIpNum);

			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "delete channel ip info failed " << ", errno:" << db->get_errno() << ", error: " << db->get_error();
			}
		}
	}

	//如果没有的话，则注册
	//开始事务
	//db->begin_transaction();
	
	/*acl::string strSalt = CNetModule::getSingleton().CreateGuidString().substr(0, 8).c_str();
	acl::string strMD5Pwd(pRegister->szPassword);
	m_MD5.reset();
	m_MD5.update(pRegister->szPassword, strlen(pRegister->szPassword));
	m_MD5.update(strSalt.c_str(), strSalt.length());
	m_MD5.finish();
	const char* pMd5 = m_MD5.get_string();*/

	//首先从帐号备库里面取出帐号ID
	/*query.reset();
	query.create_sql("SELECT userid from random_userid LIMIT :randValue, 1 FOR UPDATE")
		.set_parameter("randValue", rand() % MAX_RAND_USERID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select user id fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
		LOG(WARNING) << "sql : " << query.to_string().c_str();
		return REGISTER_UNKNOWN;
	}

	CT_DWORD dwUserID = 0;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		dwUserID = atoi((*row)["userid"]);
	}

	if (dwUserID == 0)
	{
		LOG(WARNING) << "USER ID: " << dwUserID;
	}
	db->free_result();*/
	CT_DWORD dwUserID = GetNewUserIDFromDB();

	acl::string strSalt = CNetModule::getSingleton().CreateGuidString().substr(0, 8).c_str();
	acl::string strMD5Pwd(pRegister->szPassword);
	strMD5Pwd += strSalt;
	m_MD5.reset();
	m_MD5.update(strMD5Pwd.c_str(), strMD5Pwd.length());
	m_MD5.finish();
	const char* pMd5 = m_MD5.get_string();

	query.reset();
	query.create_sql("INSERT INTO `user`(userid, promoterid, username, mobileNum, loginpass, salt, nickname, status) VALUES(:userid, 0 , :username, :moble, :loginpass, :salt, 'nickname', 1)")
		.set_format("userid", "%d", dwUserID)
		.set_format("username", "%s", pRegister->szAccount)
		.set_format("mobile", "%s", pRegister->szAccount)
		.set_format("loginpass", "%s", pMd5)
		.set_format("salt", "%s", strSalt.c_str());

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert user fail, user name: " << pRegister->szAccount << ", error = " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	/*if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		LOG(WARNING) << "select last insert id fail, error = " << db->get_errno();
		//db->rollback();
		return REGISTER_UNKNOWN;
	}

	CT_DWORD dwUserID = 0;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		dwUserID = atoi((*row)["LAST_INSERT_ID()"]);
	}
	db->free_result();*/

	//检测是否开启兑换
	accountData.cbShowExchange = GetChannelShowExchange(dwChannelID);

	//注册赠送金币
	CT_DWORD	dwRegisterPresentScore = GetChannelPresentScore(dwChannelID, en_Register);
	CT_DWORD	dwBindMobilePresentScore = GetChannelPresentScore(dwChannelID, en_BindMobile);

	//随机一个性别
	int nRandNum = rand();
	int nRandSex = nRandNum % 2 + 1;

	int nRandHeadId = nRandNum % en_HeadId_Count + en_HeadId_Boy_Start;

	query.reset();
	query.create_sql("INSERT INTO userinfo(userid, gender, headId, mobile, gem, score, bankscore, manager, promoter, regdate, regip, \
		regmachineType, regserial, lastdate, lastip, lastmachineType, lastserial, logincount, showExchange) \
		VALUES(:userid, :gender, :headId, :mobile, :gem, :score, :bankscore, 0, :promoterid, UNIX_TIMESTAMP(NOW()), :clientip, \
		:machinetype ,:serial, UNIX_TIMESTAMP(NOW()), :clientip, :machinetype , :serial, 1, :showExchange)")
		.set_format("userid", "%u", dwUserID)
		.set_format("gender", "%d", nRandSex)
		.set_format("headId", "%d", nRandHeadId)
		.set_format("mobile", "%s", pRegister->szAccount)
		.set_format("gem", "%u", 0)
		.set_format("score", "%d", dwRegisterPresentScore + dwBindMobilePresentScore)
		.set_format("bankscore", "%d", 0)
		.set_format("promoterid", "%d", dwChannelID)
		.set_format("clientip", "%s", pRegister->szClientIP)
		.set_format("machinetype", "%s", pRegister->szMachineType)
		.set_format("serial", "%s", pRegister->szMachineSerial)
		.set_format("showExchange", "%d", accountData.cbShowExchange);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert user info fail, user name: " << pRegister->szAccount << ", error: " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	//更新玩家用户昵称
	acl::string strNickName;
	//strNickName.format("玩家%u", dwUserID);
	strNickName.format("%u", dwUserID);
	query.reset();
	query.create_sql("UPDATE `user` SET nickname = :nickname WHERE userid = :userid")
		.set_format("nickname", "%s", strNickName.c_str())
		.set_format("userid", "%u", dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert user info fail, user name: " << pRegister->szAccount << ", error: " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	query.reset();
	query.create_sql("INSERT INTO userdata(userid, vip, pay) VALUES(:userid,0,0)")
		.set_format("userid", "%u", dwUserID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert user info fail, user name: " << pRegister->szAccount << ", error: " << db->get_errno();
		return LOGIN_REGISTER_FAIL;
	}

	/*query.reset();
	query.create_sql("DELETE FROM random_userid WHERE userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete user id fail. account: " << pRegister->szAccount << ", errno: " << db->get_errno() << ", error: " << db->get_errno();
		return LOGIN_REGISTER_FAIL;
	}*/

	//记录赠送金币
	InsertScoreChangeRecord(dwUserID, 0, 0, 0, dwRegisterPresentScore + dwBindMobilePresentScore, PRESENTED_SCORE);

	//返回玩家基本数据
	accountData.llScore = dwRegisterPresentScore + dwBindMobilePresentScore;
	accountData.dwUserID = dwUserID;
	accountData.cbGender = (CT_BYTE)nRandSex;
	accountData.cbHeadId = (CT_BYTE)nRandHeadId;
	accountData.dwChannelID = dwChannelID;
	_snprintf_info(accountData.szIP, sizeof(accountData.szIP), "%s", pRegister->szClientIP);
	_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", Utility::Ansi2Utf8(strNickName.c_str()).c_str());
	_snprintf_info(accountData.szPassword, sizeof(accountData.szPassword), "%s", pRegister->szPassword);
	_snprintf_info(accountData.szMobileNum, sizeof(accountData.szMobileNum), "%s", pRegister->szAccount);
	
	return REGISTER_SUCC;
}


CT_BYTE CLoginServerThread::GoDBAccountLogin(acl::db_handle* db, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData)
{
	if (db == NULL)
	{
		return LOGIN_NETBREAK;
	}

	acl::query query;
	if (pLoginData->dwUserID != 0)
	{
		query.create_sql("SELECT userid, platformId, mobileNum, loginpass, salt, status FROM `user` where userid = :userid LIMIT 1")
			.set_format("userid", "%u", pLoginData->dwUserID);
	}
	else
	{
	    //判断是否老帐号
	    std::string strFullPhoneNum(pLoginData->szAccount);
	    auto itPos = strFullPhoneNum.find("+");
	    if (itPos == std::string::npos)
        {
            strFullPhoneNum.insert(0, "(+86)");
        }

		query.create_sql("SELECT userid, platformId, mobileNum, loginpass, salt, status FROM `user` where platformId = :platformId AND mobileNum = :username LIMIT 1")
			.set_format("platformId", "%d", pLoginData->cbPlatformID)
			.set_format("username", "%s", strFullPhoneNum.c_str());
	}

	CT_DWORD dwUserID = 0;
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select account info failed, user name : " << pLoginData->szAccount << ", pid: " << (int)pLoginData->cbPlatformID
		<< ", errno: " << db->get_errno() << ", error: " << db->get_error();
		return LOGIN_NETBREAK;
	}

	//如果没有的话，就需要注册
	if (db->length() == 0)
	{
		db->free_result();
        LOG(WARNING) << "user login can not find account info c, user name : " << pLoginData->szAccount << ", pid: " << (int)pLoginData->cbPlatformID;
		return LOGIN_ACCOUNTS_NOT_EXIST;
	}
	else
	{
		int nStatus = 1;

		acl::string strSalt;
		acl::string strDBPwd;
		acl::string strMobileNum;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwUserID = atoi((*row)["userid"]);
			accountData.cbPlatformID = (CT_BYTE)atoi((*row)["platformId"]);
            strMobileNum = ((*row)["mobileNum"]);
			nStatus = atoi((*row)["status"]);
			strDBPwd = ((*row)["loginpass"]);
			strSalt = ((*row)["salt"]);
		}

		if (nStatus == en_Account_SEAL)
		{
			return LOGIN_SEAL_ACCOUNTS;
		}

		if (strMobileNum != pLoginData->szAccount)
        {
            return LOGIN_ACCOUNTS_NOT_EXIST;
        }

		//判断密码是否正确
		acl::string strLoginPwd(pLoginData->szPassword);
		strLoginPwd += strSalt;

		m_MD5.reset();
		m_MD5.update(strLoginPwd.c_str(), strLoginPwd.length());
		m_MD5.finish();
		acl::string strMd5Pwd = m_MD5.get_string();
		if (strMd5Pwd != strDBPwd)
		{
			LOG(WARNING) << "login sql: " << query.to_string().c_str();
			LOG(WARNING) << "user login fail, user name: " << pLoginData->szAccount << ", user pwd: " << pLoginData->szPassword << \
				", after add salt: " << strMd5Pwd.c_str() << ", db md5 pwd: " << strDBPwd.c_str() << ", client ip: " << pLoginData->szClientIP;
			return LOGIN_PASSWORD_ERCOR;
		}
		db->free_result();

		query.reset();
		query.create_sql("UPDATE userinfo SET lastdate = UNIX_TIMESTAMP(NOW()), lastip = :ip,\
			 lastmachineType = :machineType, lastserial = :serial, logincount = logincount+1 WHERE userid = :userid")
			.set_format("ip", "%s", pLoginData->szClientIP)
			.set_format("machineType", "%s", pLoginData->szMachineType)
			.set_format("serial", "%s", pLoginData->szMachineSerial)
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert user fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
			return LOGIN_NETBREAK;
		}

		db->free_result();
	}

	//重新读取玩家数据
	query.reset();
	query.create_sql("SELECT `user`.mobileNum, `user`.nickname, `user`.bankpass, userinfo.gender, userinfo.score, userinfo.bankscore, \
		userinfo.headId, userinfo.promoter,userinfo.showExchange, userdata.vip,userdata.vip2, userdata.pay from userinfo \
		INNER JOIN `user` ON `user`.userid = userinfo.userid \
		INNER JOIN userdata ON userdata.userid = userinfo.userid WHERE userinfo.userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "insert user fail, user name: " << pLoginData->szAccount << ", error: " << db->get_errno();
		return LOGIN_NETBREAK;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		accountData.cbGender = atoi((*row)["gender"]);
		accountData.llScore = atoll((*row)["score"]);
		accountData.llBankScore = atoll((*row)["bankscore"]);
		std::string strNickName = ((*row)["nickname"]);
		//std::string strHeadUrl = ((*row)["headimage"]);
		std::string strBankpass = ((*row)["bankpass"]);
		accountData.cbHeadId = atoi((*row)["headId"]);
		std::string strMobileNum = ((*row)["mobileNum"]);
		accountData.dwChannelID = atoi((*row)["promoter"]);
		accountData.cbVipLevel = (CT_BYTE)atoi((*row)["vip"]);
		accountData.cbVipLevel2 = (CT_BYTE)atoi((*row)["vip2"]);
		accountData.dwTotalRecharge = (CT_DWORD)atoi((*row)["pay"]);
		accountData.cbShowExchange = (CT_BYTE)atoi((*row)["showExchange"]);
		_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
		//_snprintf_info(accountData.szHeadUrl, sizeof(accountData.szHeadUrl), "%s", strHeadUrl.c_str());
		_snprintf_info(accountData.szBankPassword, sizeof(accountData.szBankPassword), "%s", strBankpass.c_str());
		_snprintf_info(accountData.szMobileNum, sizeof(accountData.szMobileNum), "%s", strMobileNum.c_str());
	}
	_snprintf_info(accountData.szPassword, sizeof(accountData.szPassword), "%s", pLoginData->szPassword);
	db->free_result();

	//读取玩家银行卡信息
	query.reset();
	query.create_sql("SELECT * FROM usercard WHERE userid = :userid")
		.set_format("userid", "%u", dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select user card fail, user id: " << dwUserID << ", error: " << db->get_errno() << ", error msg: " << db->get_error();
		return LOGIN_NETBREAK;
	}
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		std::string strAliAccount = ((*row)["aliAccount"]);
		std::string strAliName = ((*row)["aliName"]);
		std::string strBankCard = ((*row)["bankCard"]);
		std::string strBankName = ((*row)["bankAccount"]);
		_snprintf_info(accountData.szAliPayAccout, sizeof(accountData.szAliPayAccout), "%s", Utility::Ansi2Utf8(strAliAccount).c_str());
		_snprintf_info(accountData.szAliPayName, sizeof(accountData.szAliPayName), "%s", Utility::Ansi2Utf8(strAliName).c_str());
		_snprintf_info(accountData.szBankCardNum, sizeof(accountData.szBankCardNum), "%s", Utility::Ansi2Utf8(strBankCard).c_str());
		_snprintf_info(accountData.szBankCardName, sizeof(accountData.szBankCardName), "%s", Utility::Ansi2Utf8(strBankName).c_str());
	}
	db->free_result();

	accountData.dwUserID = dwUserID;
	return 0;
}

CT_BYTE CLoginServerThread::GoDBBindMobile(acl::db_handle* db, MSG_CS_Bind_Mobile* pBindAccount, std::string& oldAccount, acl::string& newNickName, CT_BYTE& cbPlatformId)
{
	if (db == NULL)
	{
		return REGISTER_UNKNOWN;
	}

	acl::query query;
	query.create_sql("SELECT platformId, username FROM `user` where userid = :userid LIMIT 1")
		.set_format("userid", "%u", pBindAccount->dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "bind account failed, user id : " << pBindAccount->dwUserID << ", error: " << db->get_errno();
		return LOGIN_ACCOUNTS_NOT_EXIST;
	}

	if (db->length() == 0)
	{
		db->free_result();
		return LOGIN_ACCOUNTS_NOT_EXIST;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		cbPlatformId = (CT_BYTE)atoi((*row)["platformId"]);
		oldAccount = (*row)["username"];
	}
	db->free_result();

	//判断数据库里面有没有这个手机号绑定
	query.reset();
	query.create_sql("SELECT * FROM `user` where platformId = :platformId AND mobileNum = :username LIMIT 1")
		.set_format("platformId", "%d", cbPlatformId)
		.set_format("username", "%s", pBindAccount->szMobileNum);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "check account exist failed, user name : " << pBindAccount->szMobileNum << ", pid: " << (int)cbPlatformId
		<< ", errno: " << db->get_errno() << ", error: " << db->get_error();
		return LOGIN_NETBREAK;
	}

	if (db->length() != 0)
	{
		db->free_result();
		return REGISTER_ACCOUNT_REPEAT;
	}
	db->free_result();

	//判断原来的帐号是否已经绑定
	/*query.reset();
	query.create_sql("SELECT mobile FROM userinfo where userid = :userid LIMIT 1")
		.set_format("userid", "%u", pBindAccount->dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "check is bind mobile failed, user id : " << pBindAccount->dwUserID << ", error: " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	std::string isBindMobile;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		isBindMobile = (*row)["mobile"];
	}
	db->free_result();
	if (isBindMobile.length() > 1)
	{
		LOG(WARNING) << "user has bind mobile, want to rebind , fuck you. user id : " << pBindAccount->dwUserID;
		return REGISTER_ACCOUNT_REPEAT;
	}*/

	acl::string strSalt = CNetModule::getSingleton().CreateGuidString().substr(0, 8).c_str();
	acl::string strMD5Pwd(pBindAccount->szPassword);
	strMD5Pwd += strSalt;

	m_MD5.reset();
	m_MD5.update(strMD5Pwd.c_str(), strMD5Pwd.length());
	m_MD5.finish();
	const char* pMd5 = m_MD5.get_string();

	//更新玩家名称(实际是玩家手机号)
	query.reset();
	query.create_sql("UPDATE `user` SET mobileNum = :username,  loginpass= :loginpass, salt = :salt, nickname = :nickname where userid = :userid LIMIT 1")
		.set_format("userid", "%u", pBindAccount->dwUserID)
		.set_format("username", "%s", pBindAccount->szMobileNum)
		.set_format("loginpass", "%s", pMd5)
		.set_format("salt", "%s", strSalt.c_str())
		.set_format("nickname", "%s", newNickName.c_str());

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "bind user fail, user name: " << pBindAccount->szMobileNum << ", error = " << db->get_errno();
		return REGISTER_UNKNOWN;
	}

	//更新玩家手机号
	/*query.reset();
	query.create_sql("UPDATE userinfo SET mobile = :mobile where userid = :userid LIMIT 1")
		.set_format("userid", "%u", pBindAccount->dwUserID)
		.set_format("mobile", "%s", pBindAccount->szMobileNum);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "bind user mobile fail, user name: " << pBindAccount->szMobileNum << ", error = " << db->get_errno();
		return REGISTER_UNKNOWN;
	}
	db->free_result();
	 */

	return REGISTER_SUCC;
}

CT_BYTE CLoginServerThread::GoDBCheckMobileBind(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return REGISTER_UNKNOWN;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return REGISTER_UNKNOWN;
	}

	bool bSucc = true;
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM `user` where platformId = :platformId AND mobileNum = :username LIMIT 1")
			.set_format("platformId", "%d", cbPlatformId)
			.set_format("username", "%s", pMobileNum);

		//LOG(WARNING) << "GoDBCheckMobileBind: " << query.to_string().c_str();
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "check account exist failed, user name : " << pMobileNum << ", errno: " << db->get_errno() << ", error: " << db->get_error();
			bSucc = false;
			break;
		}

		if (db->length() != 0)
		{
			db->free_result();
			bSucc = false;
			break;
		}
		db->free_result();

	} while (0);

	pool->put(db);

	if (bSucc)
		return 0;

	return REGISTER_UNKNOWN;
}

CT_BYTE CLoginServerThread::GoDBResetPwd(MSG_CS_Reset_PwdEx* pResetPwd)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return REGISTER_UNKNOWN;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return REGISTER_UNKNOWN;
	}

	bool bSucc = true;
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM `user` where platformId = :platformId AND mobileNum = :username LIMIT 1")
		    .set_format("platformId", "%d", pResetPwd->cbPlatformId)
			.set_format("username", "%s", pResetPwd->szMobileNum);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "check account exist failed, user name : " << pResetPwd->szMobileNum << ", error: " << db->get_errno();
			bSucc = false;
			break;
		}

		if (db->length() == 0)
		{
			db->free_result();
			bSucc = false;
			break;
		}

		acl::string strSalt = CNetModule::getSingleton().CreateGuidString().substr(0, 8).c_str();
		acl::string strMD5Pwd(pResetPwd->szPassword);
		strMD5Pwd += strSalt;

		m_MD5.reset();
		m_MD5.update(strMD5Pwd.c_str(), strMD5Pwd.length());
		m_MD5.finish();
		const char* pMd5 = m_MD5.get_string();

		//更新玩家名称(实际是玩家手机号)
		query.reset();
		query.create_sql("UPDATE `user` SET loginpass= :loginpass, salt = :salt where platformId = :platformId AND mobileNum = :username LIMIT 1")
		    .set_format("platformId", "%d", pResetPwd->cbPlatformId)
			.set_format("username", "%s", pResetPwd->szMobileNum)
			.set_format("loginpass", "%s", pMd5)
			.set_format("salt", "%s", strSalt.c_str());

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "reset user pwd fail, user name: " << pResetPwd->szMobileNum << ", error = " << db->get_errno();
			bSucc = false;
			break;
		}

	} while (0);

	pool->put(db);

	if (bSucc)
		return 0;

	return REGISTER_UNKNOWN;
}

CT_BYTE CLoginServerThread::GoDBModifyPersonalInfo(MSG_CS_Modify_PersonalInfo* pModifyInfo)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return REGISTER_UNKNOWN;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return REGISTER_UNKNOWN;
	}

	bool bSucc = true;
	do
	{
		acl::query query;
		//更新玩家个人新信息
		query.create_sql("UPDATE userinfo SET gender = :gender, headId = :headId where userid = :userid LIMIT 1")
			.set_format("gender", "%d", pModifyInfo->cbSex)
			.set_format("headId", "%d", pModifyInfo->cbHeadId)
			.set_format("userid", "%u", pModifyInfo->dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user personal info fail, user id: " << pModifyInfo->dwUserID << ", error = " << db->get_errno();
			bSucc = false;
			break;
		}

	} while (0);

	pool->put(db);

	if (bSucc)
		return 0;

	return REGISTER_UNKNOWN;
}

CT_BYTE	CLoginServerThread::GoDBBindExchangeAliPay(MSG_CS_Bind_AliPay* pBindAliPay)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return REGISTER_UNKNOWN;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return REGISTER_UNKNOWN;
	}

	bool bSucc = true;
	do
	{
		acl::query query;
		//更新玩家个人新信息
		query.create_sql("INSERT INTO usercard(userid, aliAccount, aliName, bankAccount, bankCard, bankType, bankAddress) \
				VALUES (:userid, :aliPay, :aliName,  '', '', 0, '') ON DUPLICATE KEY UPDATE aliAccount = :aliPay, aliName = :aliName")
			.set_format("userid", "%d", pBindAliPay->dwUserID)
			.set_format("aliPay", "%s", pBindAliPay->szAliPayAccount)
			.set_format("aliName", "%s", Utility::Utf82Ansi(pBindAliPay->szRealName).c_str());

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update AliPay fail, user id: " << pBindAliPay->dwUserID << ", error = " << db->get_errno() << ", error msg: " << db->get_error();
			bSucc = false;
			break;
		}

	} while (0);

	pool->put(db);

	if (bSucc)
		return 0;

	return REGISTER_UNKNOWN;
}

CT_BYTE	CLoginServerThread::GoDBBindExchangeBankCard(MSG_CS_Bind_BankCard* pBankCard)
{
	acl::db_pool* pool = GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return REGISTER_UNKNOWN;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return REGISTER_UNKNOWN;
	}

	bool bSucc = true;
	do
	{
		acl::query query;
		//更新玩家个人新信息
		query.create_sql("INSERT INTO usercard(userid, aliAccount, aliName, bankAccount, bankCard, bankType, bankAddress) \
				VALUES (:userid, '', '',  :bankAccount, :bankCard, :bankType, :bankAddress) ON DUPLICATE KEY UPDATE \
				bankAccount = :bankAccount, bankCard = :bankCard, bankType= :bankType,bankAddress=:bankAddress")
			.set_format("userid", "%d", pBankCard->dwUserID)
			.set_format("bankAccount", "%s", Utility::Utf82Ansi(pBankCard->szRealName).c_str())
			.set_format("bankCard", "%s", pBankCard->szBankCard)
			.set_format("bankType", "%d", pBankCard->cbBankCardType)
			.set_format("bankAddress", "%s", Utility::Utf82Ansi(pBankCard->szBankAddress).c_str());

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update bank card fail, user id: " << pBankCard->dwUserID << ", error = " << db->get_errno() << ", error msg: " << db->get_error();
			bSucc = false;
			break;
		}

	} while (0);
	pool->put(db);

	if (bSucc)
		return 0;

	return REGISTER_UNKNOWN;
}

CT_DWORD CLoginServerThread::GetNewUserIDFromDB()
{
	CT_DWORD dwUserID = 0;
	acl::db_handle* db = m_accountDBPool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "new user id from db fail, get db handle null.";
		return 0;
	}

	do 
	{
		if (db->begin_transaction() == false)
		{
			LOG(WARNING) << "new user id from db, begin transaction fail. errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		acl::query query;
		query.create_sql("SELECT userid from random_userid LIMIT :randValue, 1 FOR UPDATE")
			.set_parameter("randValue", rand() % MAX_RAND_USERID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select user id fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			db->rollback();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwUserID = atoi((*row)["userid"]);
		}
		db->free_result();

		query.reset();
		query.create_sql("DELETE FROM random_userid WHERE userid = :userid")
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "delete user id fail. errno: " << db->get_errno() << ", error: " << db->get_errno();
			dwUserID = 0;
			db->rollback();
			break;
		}

		//提交事务
		if (db->commit() == false)
		{
			LOG(WARNING) << "new user id commit fail. errno: " << db->get_errno() << ", error: " << db->get_error();
			dwUserID = 0;
			db->rollback();
			break;
		}
	} while (0);

	m_accountDBPool->put(db);

	return dwUserID;
}

CT_WORD	CLoginServerThread::GetIpRegisterCount(CT_DWORD dwIp)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return 0;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return 0;
	}

	CT_WORD wRegisterCount = 0;
	do
	{
		acl::query query;
		query.create_sql("SELECT count FROM record_register_ip WHERE ip = :ip")
			.set_format("ip", "%u", dwIp);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "GetIpRegisterCount error" << ", error = " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			wRegisterCount = (CT_WORD) atoi((*row)["count"]);
		}

		db->free_result();

	} while (0);


	pool->put(db);

	return wRegisterCount;
}

CT_VOID	CLoginServerThread::IncreaseRegisterCount(CT_DWORD dwIp)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	//CT_WORD wRegisterCount = 0;
	do
	{
		acl::query query;
		query.create_sql("INSERT INTO record_register_ip(ip, count) VALUES(:ip, 1) ON DUPLICATE KEY UPDATE count = count + 1")
			.set_format("ip", "%u", dwIp);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "IncreaseRegisterCount error" << ", error = " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}
	} while (0);

	pool->put(db);
}

CT_VOID CLoginServerThread::InitServer()
{
	m_redisClient.set_password(CServerCfg::m_RedisPwd);
	m_redis.set_client(&m_redisClient);

    m_redisClient2.set_password(CServerCfg::m_RedisPwd2);
    m_redis2.set_client(&m_redisClient2);

	CNetModule::getSingleton().SetTimer(IDI_TIMER_INIT_SERVER, TIMER_INIT_SERVER, this, false);

	//CNetModule::getSingleton().SetTimer(IDI_TIMER_TEST, 2000, this, true);
}

/*CT_BOOL CLoginServerThread::ConnectAccountDB()
{
	CT_BOOL ret = m_accountDBCon.Connect(CServerCfg::m_AccountdbIP, 
										CServerCfg::m_AccountdbUser, 
										CServerCfg::m_AccountdbPwd,
										CServerCfg::m_AccountdbName,
										CServerCfg::m_AccountdbPort); 

	if (!ret)
	{
		LOG(ERROR) << "connect account db failed!";
	}
	else
	{
		CNetModule::getSingleton().KillTimer(IDI_TIMER_ACCOUNT_DB_RECONNECT);
	}

	return ret;
}

CT_VOID CLoginServerThread::OnAccountDBError()
{
	if (m_accountDBCon.GetErrno() == DB_NET_ERROR)
	{
		CNetModule::getSingleton().SetTimer(IDI_TIMER_ACCOUNT_DB_RECONNECT, TIME_DB_RECONNECT, this, true);
	}
}*/

/*CT_BOOL CLoginServerThread::ConnectGamePHZDB()
{
	CT_BOOL ret = m_gamePHZDBCon.Connect(CServerCfg::m_GamePHZdbIP,
		CServerCfg::m_GamePHZdbUser,
		CServerCfg::m_GamePHZdbPwd,
		CServerCfg::m_GamePHZdbName,
		CServerCfg::m_GamePHZdbPort);

	if (!ret)
	{
		LOG(ERROR) << "connect game_phz db failed!";
	}
	else
	{
		CNetModule::getSingleton().KillTimer(IDI_TIMER_GAME_PHZ_DB_RECONNECT);
	}

	return ret;
}

CT_VOID CLoginServerThread::OnGamePHZDBError()
{
	if (m_gamePHZDBCon.GetErrno() == DB_NET_ERROR)
	{
		CNetModule::getSingleton().SetTimer(IDI_TIMER_GAME_PHZ_DB_RECONNECT, TIME_DB_RECONNECT, this, true);
	}
}*/

/*CT_BOOL CLoginServerThread::ConnectPlatformDB()
{
	CT_BOOL ret = m_platformDBCon.Connect(CServerCfg::m_PlatformdbIP,
		CServerCfg::m_PlatformdbUser,
		CServerCfg::m_PlatformdbPwd,
		CServerCfg::m_PlatformdbName,
		CServerCfg::m_PlatformdbPort);

	if (!ret)
	{
		LOG(ERROR) << "connect platform db failed!";
	}
	else
	{
		CNetModule::getSingleton().KillTimer(IDI_TIMER_PLATFORM_DB_RECONNECT);
	}

	return ret;
}

CT_VOID CLoginServerThread::OnPlatformDBError()
{
	if (m_platformDBCon.GetErrno() == DB_NET_ERROR)
	{
		CNetModule::getSingleton().SetTimer(IDI_TIMER_PLATFORM_DB_RECONNECT, TIME_DB_RECONNECT, this, true);
	}
}*/

/*void CLoginServerThread::ExeNoThingProc()
{
	m_accountDBCon.Reset();
	m_accountDBCon.SetPcName("G_OUT_ExeNoThingOP");
	if (m_accountDBCon.Exec())
	{
		CCDBResult* pResult = m_accountDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			pResult->Release();
		}
	}
	else
	{
		OnAccountDBError();
	}

	m_platformDBCon.Reset();
	m_platformDBCon.SetPcName("G_OUT_ExeNoThingOP");
	if (m_platformDBCon.Exec())
	{
		CCDBResult* pResult = m_platformDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			pResult->Release();
		}
	}
	else
	{
		OnPlatformDBError();
	}

	m_gamePHZDBCon.Reset();
	m_gamePHZDBCon.SetPcName("G_OUT_ExeNoThingOP");
	if (m_gamePHZDBCon.Exec())
	{
		CCDBResult* pResult = m_gamePHZDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			pResult->Release();
		}
	}
	else
	{
		OnGamePHZDBError();
	}
}*/

CT_VOID CLoginServerThread::LoadGameKind(acl::db_handle* db)
{
	/*
	m_platformDBCon.Reset();
	m_platformDBCon.SetPcName("G_OUT_LoadGameKindItem");
	if (m_platformDBCon.Exec())
	{
		CCDBResult* pResult = m_platformDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			while (pResult->MoveNextRow())
			{
				tagGameKind *pGameKind = new tagGameKind;

				pResult->GetValue(0, pGameKind->wGameID);
				pResult->GetValue(1, pGameKind->wKindID);
				pResult->GetValue(2, pGameKind->wSortID);
				pResult->GetValue(3, pGameKind->szServerDll, PROCESSNAME_STR_LEN);
				std::string strKindName;
				pResult->GetValue(4, strKindName);
				std::string strUtf8KindName = Utility::Ansi2Utf8(strKindName);
				_snprintf_info(pGameKind->szKindName, sizeof(pGameKind->szKindName), "%s", strUtf8KindName.c_str());
				CServerMgr::get_instance().InsertGameKind(pGameKind);
			}
			pResult->Release();
		}
	}
	else
	{
		OnPlatformDBError();
		return;
	}
	*/

	acl::query query;
	query.create_sql("select * from gamekind");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game kind fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		tagGameKind *pGameKind = new tagGameKind;
		const acl::db_row* row = (*db)[i];
		pGameKind->wGameID = (CT_WORD)atoi((*row)["gameid"]);
		pGameKind->wKindID = (CT_WORD)atoi((*row)["kindid"]);
		pGameKind->wSortID = (CT_WORD)atoi((*row)["sortid"]);
		std::string strServerDllName = (*row)["serverDllName"];
		std::string strKindName = (*row)["kindname"];
		_snprintf_info(pGameKind->szServerDll, sizeof(pGameKind->szServerDll), "%s", Utility::Ansi2Utf8(strServerDllName).c_str());
		_snprintf_info(pGameKind->szKindName, sizeof(pGameKind->szKindName), "%s", Utility::Ansi2Utf8(strKindName).c_str());
		CServerMgr::get_instance().InsertGameKind(pGameKind);
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load game kind succ!";
	}
	else
	{
		LOG(WARNING) << "load game kind failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadGameRoomKind(acl::db_handle* db)
{
	/*m_platformDBCon.Reset();
	m_platformDBCon.SetPcName("G_OUT_LoadGameRoomKindItem");
	if (m_platformDBCon.Exec())
	{
		CCDBResult* pResult = m_platformDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			while (pResult->MoveNextRow())
			{
				tagGameRoomKind *pGamePlayKind = new tagGameRoomKind;

				pResult->GetValue(0, pGamePlayKind->wGameID);
				pResult->GetValue(1, pGamePlayKind->wKindID);
				pResult->GetValue(2, pGamePlayKind->wRoomKindID);
				std::string strRoomKindName;
				pResult->GetValue(3, strRoomKindName);
				std::string strUtf8RoomKindName = Utility::Ansi2Utf8(strRoomKindName);
				_snprintf_info(pGamePlayKind->szRoomKindName, sizeof(pGamePlayKind->szRoomKindName), "%s", strUtf8RoomKindName.c_str());
				pResult->GetValue(4, pGamePlayKind->wSortID);
				pResult->GetValue(5, pGamePlayKind->dwCellScore);
				pResult->GetValue(6, pGamePlayKind->wTableCount);
				pResult->GetValue(7, pGamePlayKind->wStartMinPlayer);
				pResult->GetValue(8, pGamePlayKind->wStartMaxPlayer);
				pResult->GetValue(9, pGamePlayKind->dwEnterMinScore);
				pResult->GetValue(10, pGamePlayKind->dwEnterMaxScore);
				pResult->GetValue(11, pGamePlayKind->wEnterVipLevel);
				pResult->GetValue(12, pGamePlayKind->dwRevenue);
				pResult->GetValue(13, pGamePlayKind->cbCreateRoomPlayCount1);
				pResult->GetValue(14, pGamePlayKind->wCreateRoomNeedGem1);
				pResult->GetValue(15, pGamePlayKind->wCreateRoomHuxi1);
				pResult->GetValue(16, pGamePlayKind->cbCreateRoomPlayCount2);
				pResult->GetValue(17, pGamePlayKind->wCreateRoomNeedGem2);
				pResult->GetValue(18, pGamePlayKind->wCreateRoomHuxi2);

				CServerMgr::get_instance().InsertGamePlayKind(pGamePlayKind);
			}
			pResult->Release();
		}
	}
	else
	{
		OnPlatformDBError();
		return;
	}*/

	acl::query query;
	query.create_sql("SELECT * FROM gameroomkind");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game room kind fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		tagGameRoomKind *pGamePlayKind = new tagGameRoomKind;

		const acl::db_row* row = (*db)[i];
		pGamePlayKind->wGameID = (CT_WORD)atoi((*row)["gameid"]);
		pGamePlayKind->wKindID = (CT_WORD)atoi((*row)["kindid"]);
		pGamePlayKind->wRoomKindID = (CT_WORD)atoi((*row)["roomKindid"]);
		std::string strRoomKindName = (*row)["roomKindname"];
		_snprintf_info(pGamePlayKind->szRoomKindName, sizeof(pGamePlayKind->szRoomKindName), "%s", Utility::Ansi2Utf8(strRoomKindName).c_str());
		//pGamePlayKind->wSortID = (CT_WORD)atoi((*row)["sortid"]);
		pGamePlayKind->dwCellScore = (CT_DWORD)atoi((*row)["cellScore"]);
		pGamePlayKind->wTableCount = (CT_WORD)atoi((*row)["tableCount"]);
		pGamePlayKind->wStartMinPlayer = (CT_WORD)atoi((*row)["startMinPlayer"]);
		pGamePlayKind->wStartMaxPlayer = (CT_WORD)atoi((*row)["startMaxPlayer"]);
		pGamePlayKind->dwEnterMinScore = (CT_DWORD)atoi((*row)["enterMinScore"]);
		pGamePlayKind->dwEnterMaxScore = (CT_DWORD)atoi((*row)["enterMaxScore"]);
		//pGamePlayKind->dwCellScore	= (CT_DWORD)atoi((*row)["cellScore"]);
		pGamePlayKind->wEnterVipLevel = (CT_WORD)atoi((*row)["enterVipLevel"]);
		pGamePlayKind->dwRevenue = (CT_DWORD)atoi((*row)["revenue"]);
		pGamePlayKind->dwMaxUserCount = (CT_DWORD)atoi((*row)["maxUserCount"]);//服务器承载最大人数
		pGamePlayKind->iBroadcastScore = atoi((*row)["broadcastScore"]);
		std::string strPRoomOtherCond = (*row)["proomOtherCond"];
		_snprintf_info(pGamePlayKind->szPRoomCond, sizeof(pGamePlayKind->szPRoomCond), "%s", Utility::Ansi2Utf8(strPRoomOtherCond).c_str());

		acl::json proomJson(pGamePlayKind->szPRoomCond);
		const acl::json_node* pNodePlayCount = proomJson.getFirstElementByTagName("playcount");
		std::string  strPlayCount;
		if (pNodePlayCount)
		{
			strPlayCount = pNodePlayCount->get_string();
		}

		const acl::json_node* pNodeNeedGem = proomJson.getFirstElementByTagName("needgem");
		std::string strNeedGem;
		if (pNodeNeedGem)
		{
			strNeedGem = pNodeNeedGem->get_string();
		}

		CServerMgr::get_instance().InsertGamePlayKind(pGamePlayKind);
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load game room kind succ!";
	}
	else
	{
		LOG(WARNING) << "load game room kind failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadSignInConfig(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT dayID, rewardGem, rewardScore FROM signinconfig");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query sign in config fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_INT32 nDayID;
		stSignInReward signInReward;
		nDayID = atoi((*row)["dayID"]);
		signInReward.wRewardGem = atoi((*row)["rewardGem"]);
		signInReward.dwRewardScore = atoi((*row)["rewardScore"]);

		m_mapsignInConfig.insert(std::make_pair(nDayID, signInReward));
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load sign in config succ!";
	}
	else
	{
		LOG(WARNING) << "load sign in config failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadVipConfig(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT level, signin FROM vipconfig");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query vip config fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_WORD nLevel;
		stVipReward vipReward;
		nLevel = atoi((*row)["level"]);
		vipReward.dwSignInRewardScore = atoi((*row)["signin"]);

		m_mapVipReward.insert(std::make_pair(nLevel, vipReward));
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load vip config succ!";
	}
	else
	{
		LOG(WARNING) << "load vip in config failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadVip2Config(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM vip2config");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select vip2 config fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_BYTE nLevel;
		CT_DWORD dwCash;
		nLevel = (CT_BYTE)atoi((*row)["level"]);
		dwCash = atoi((*row)["cash"]);

		m_mapVip2Level.insert(std::make_pair(nLevel, dwCash));
	}

	LOG(INFO) << "load vip2 config succ!";
	db->free_result();
}

CT_VOID	CLoginServerThread::LoadBenefitConfig(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM benefit_config");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query benefit config fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
        tagBenefitReward benefitReward;
        benefitReward.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
        benefitReward.cbRewardCount = (CT_BYTE)atoi((*row)["totalCount"]);
        benefitReward.dwRewardScore = atoi((*row)["rewardScore"]);
        benefitReward.dwLessScore = atoi((*row)["lessScore"]);
        m_mapBenefitConfig.insert(std::make_pair(benefitReward.cbVip2, benefitReward));
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load benefit config succ!";
	}
	else
	{
		LOG(WARNING) << "load benefit config failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadExchangeScoreConfig(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM exchangescoreconfig");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query exchange score config fail.";
		return;
	}
	m_mapExchageScoreCfg.clear();
	tagExchangeScoreConfig exchangeScoreCfg;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		exchangeScoreCfg.wGem = atoi((*row)["gem"]);
		exchangeScoreCfg.dwExchangeScore = atoi((*row)["exchangeScore"]);
		exchangeScoreCfg.dwExtraPresentScore = atoi((*row)["presentScore"]);

		m_mapExchageScoreCfg.insert(std::make_pair(exchangeScoreCfg.wGem, exchangeScoreCfg));
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load exchange score config succ!";
	}
	else
	{
		LOG(WARNING) << "load exchange score config failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadGameVersion(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM gameversion");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game version fail.";
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_WORD wGameID = atoi((*row)["gameid"]);
		CT_WORD wVersionID = atoi((*row)["version"]);
		//stGameVersion gameVersion;
		//GetGameVersion(strVersion, gameVersion);
		m_mapGameVersion.insert(std::make_pair(wGameID, wVersionID));
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load game version succ!";
	}
	else
	{
		LOG(INFO) << "load game version failed!";
	}

	db->free_result();
}

CT_VOID CLoginServerThread::LoadRegisterReward(acl::db_handle* db)
{
	//注册送钻石
	acl::query query;
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'registerGem'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select register gem fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_iRegisterPresentGem = atoi((*row)["statusValue"]);
	}
	db->free_result();

	//注册送金币
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'registerScore'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select register score fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_iRegisterPresentScore = atoi((*row)["statusValue"]);
	}
	db->free_result();

	//绑定手机送金币
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'bindMobile'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select register score fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_iBindMobileScore = atoi((*row)["statusValue"]);
	}
	db->free_result();
}

CT_VOID CLoginServerThread::LoadSystemStatus(acl::db_handle* db)
{
	acl::query query;

	//签到状态
	/*query.reset();
	query.create_sql("SELECT StatusValue from systemStatus where statusName = 'signInStatus'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select signin status fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_cbSignInStatus = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();

	//救济金状态
	query.reset();
	query.create_sql("SELECT StatusValue from systemStatus where statusName = 'benefitStatus'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select benefit status fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_cbBenefitStatus = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();*/

	//金币场信息
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'scoreMode'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select load score info fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_cbScoreMode = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();

	//私人场信息
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'privateMode'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select load score info fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_cbPrivateMode = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();

	//是否打开苹果充值
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'openApplePay'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select load score info fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_cbOpenApplePay = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();

	//IP注册限制
	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'ipRegisterCount'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select ip register count fail, error: " << db->get_errno();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_wIpRegisterLimite = (CT_WORD)atoi((*row)["statusValue"]);
	}
	db->free_result();

	query.reset();
	query.create_sql("SELECT statusValue from systemstatus where statusName = 'smsPid'");
    if (db->exec_select(query) == false)
    {
        LOG(WARNING) << "select smsPid fail, error: " << db->get_errno();
        return;
    }

    for (size_t i = 0; i < db->length(); i++)
    {
        const acl::db_row* row = (*db)[i];
        CVerificationCodeThread::m_dwUseIndex = (CT_DWORD)atoi((*row)["statusValue"]);
    }
    db->free_result();

	LOG(INFO) << "load system status succ!";
}

CT_VOID CLoginServerThread::LoadChannelShowExchange(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM treasure.channel WHERE type = 1 AND showExchangeCond != 0");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select channel show exchange condition fail, errno:" << db->get_errno() << ", error: " << db->get_error();
		return;
	}

	tagChannelShowExchangeCond channelShowExchange;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&channelShowExchange, 0, sizeof(channelShowExchange));
		const acl::db_row* row = (*db)[i];
		channelShowExchange.dwChannelID = atoi((*row)["id"]);
		channelShowExchange.dwTotalRecharge = atoi((*row)["showExchangeCond"]);
		m_mapChannelShowExchange.insert(std::make_pair(channelShowExchange.dwChannelID, channelShowExchange));
	}
	db->free_result();

	LOG(INFO) << "load channel show exchange succ!";
}

CT_VOID CLoginServerThread::LoadExchangeInfo(acl::db_handle* db)
{
    acl::query query;
    query.create_sql("SELECT * FROM treasure.exchange_amount_cfg ORDER BY type");
    
    if (db->exec_select(query) == false)
    {
        LOG(WARNING) << "select excharge amount cfg fail, errno:" << db->get_errno() << ", error: " << db->get_error();
        return;
    }
	
	tagExchangeChannelAmount exchangeChannelAmount;
    for (size_t i = 0; i < db->length(); i++)
    {
        const acl::db_row* row = (*db)[i];
        exchangeChannelAmount.cbChannelID = (CT_BYTE)atoi((*row)["type"]);
        exchangeChannelAmount.dwAmount[0] = (CT_DWORD)atoi((*row)["amount1"]);
        exchangeChannelAmount.dwAmount[1] = (CT_DWORD)atoi((*row)["amount2"]);
        exchangeChannelAmount.dwAmount[2] = (CT_DWORD)atoi((*row)["amount3"]);
        exchangeChannelAmount.dwAmount[3] = (CT_DWORD)atoi((*row)["amount4"]);
        exchangeChannelAmount.dwAmount[4] = (CT_DWORD)atoi((*row)["amount5"]);
        exchangeChannelAmount.dwMinAmount = (CT_DWORD)atoi((*row)["minAmount"]);
        exchangeChannelAmount.dwMaxAmount = (CT_DWORD)atoi((*row)["maxAmount"]);
        exchangeChannelAmount.cbDayCountLimite = (CT_BYTE)atoi((*row)["dayCountLimit"]);
        exchangeChannelAmount.wHandlingFee = (CT_WORD)atoi((*row)["handlingFee"]);
        m_vecExchangeInfo.push_back(exchangeChannelAmount);
    }
    LOG(INFO) << "load exchange info succ!";
}

CT_VOID CLoginServerThread::LoadChannelPresentScore(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT id,initScore,bindScore FROM treasure.channel WHERE type = 1 ");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select channel present score fail, errno:" << db->get_errno() << ", error: " << db->get_error();
		return;
	}

	stChannelPresentScore presentScore;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&presentScore, 0, sizeof(presentScore));
		const acl::db_row* row = (*db)[i];

		CT_WORD wChannelID = atoi((*row)["id"]);
		presentScore.dwRegisterPresentScore = atoi((*row)["initScore"]);
		presentScore.dwBindMobilePresentScore = atoi((*row)["bindScore"]);
		m_mapChannelPresentScore.insert(std::make_pair(wChannelID, presentScore));
	}
	db->free_result();

	LOG(INFO ) << "load channel prsent score succ!";
}

CT_VOID CLoginServerThread::LoadChannelSendRegisterMail(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT id,enableMail FROM treasure.channel WHERE type = 1 AND enableMail = 1");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select channel register mail fail, errno:" << db->get_errno() << ", error: " << db->get_error();
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];

		CT_WORD wChannelID = atoi((*row)["id"]);
		CT_BYTE cbRegisterMail = (CT_BYTE)atoi((*row)["enableMail"]);
		m_mapChannelRegisterMail.insert(std::make_pair(wChannelID, cbRegisterMail));
	}
	db->free_result();

	LOG(INFO) << "load channel register mail succ!";
}


/*CT_VOID	CLoginServerThread::InsertUserID()
{
	acl::db_pool* accountPool = GetAccountdbPool();
	if (accountPool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* accountdb = accountPool->peek_open();
	if (accountdb == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	do 
	{
		acl::query query;

		acl::string strSql;
		strSql.format("INSERT INTO account.random_userid(userid) VALUES (%d)", 80000000);

		acl::string strSqlEx;
		CT_DWORD dwTime = Utility::GetTick();
		for (int i = 80000001; i < 80010000; ++i)
		{
			strSqlEx.format(",(%d)", i);
			strSql.append(strSqlEx);
		}
		
		if (!accountdb->sql_update(strSql.c_str()))
		{
			LOG(WARNING) << "insert user id fail!";
			break;
		}

		LOG(WARNING) << "user time: " << (Utility::GetTick() - dwTime);

	} while (0);

	
	accountPool->put(accountdb);
}*/

CT_VOID CLoginServerThread::GetGameVersion(std::string strVersion, stGameVersion& version)
{
	auto pos1 = strVersion.find_first_of(".");
	auto pos2 = strVersion.find_last_of(".");

	
	if (pos1 != std::string::npos)
	{
		version.wMainVer = atoi(strVersion.substr(0, pos1).c_str());
	}

	if (pos2 != std::string::npos)
	{
		version.wSubVer = atoi(strVersion.substr(pos1 + 1, pos2).c_str());
		version.wBuildVer = atoi(strVersion.substr(pos2 + 1).c_str());
	}
}

CT_VOID CLoginServerThread::InsertLoginRecord(CT_DWORD dwUserID, MSG_CS_LoginDataEx* pLogin)
{
	std::shared_ptr<tagUserLoginInfo> LoginInfoPtr = m_LoginRecordThreadPtr->GetFreeLoginInfoQue();
	LoginInfoPtr->dwUserID = dwUserID;
	_snprintf_info(LoginInfoPtr->szLoginSerial, sizeof(LoginInfoPtr->szLoginSerial), "%s", pLogin->szMachineSerial);
	_snprintf_info(LoginInfoPtr->szloginMachineType, sizeof(LoginInfoPtr->szloginMachineType), "%s", pLogin->szMachineType);
	_snprintf_info(LoginInfoPtr->szLoginIp, sizeof(LoginInfoPtr->szLoginIp), "%s", pLogin->szClientIP);

	m_LoginRecordThreadPtr->InsertLoginInfo(LoginInfoPtr);
}

CT_BYTE CLoginServerThread::GetChannelShowExchange(CT_DWORD dwChannelID)
{
	auto it = m_mapChannelShowExchange.find(dwChannelID);
	if (it != m_mapChannelShowExchange.end())
	{
		return it->second.dwTotalRecharge > 0 ? 0 : 1;
	}

	return 1;
}

CT_DWORD CLoginServerThread::GetChannelShowExchangeRecharge(CT_DWORD dwChannelID)
{
	auto it = m_mapChannelShowExchange.find(dwChannelID);
	if (it != m_mapChannelShowExchange.end())
	{
		return it->second.dwTotalRecharge;
	}

	return  0;
}

//查找赠送金币
CT_DWORD CLoginServerThread::GetChannelPresentScore(CT_WORD wChannelID, enPresentMode cbMode)
{
	if (cbMode == en_Register)
	{
		auto it = m_mapChannelPresentScore.find(wChannelID);
		if (it != m_mapChannelPresentScore.end())
		{
			return it->second.dwRegisterPresentScore;
		}

		return m_iRegisterPresentScore;
	}
	else
	{
		auto it = m_mapChannelPresentScore.find(wChannelID);
		if (it != m_mapChannelPresentScore.end())
		{
			return it->second.dwBindMobilePresentScore;
		}

		return m_iBindMobileScore;
	}
}

//登陆发送注册邮件
CT_BOOL CLoginServerThread::IsChannelSendRegisterMail(CT_WORD wChannelID)
{
	auto it = m_mapChannelRegisterMail.find(wChannelID);
	if (it != m_mapChannelRegisterMail.end())
	{
		return it->second == 1;
	}

	return false;
}

CT_VOID CLoginServerThread::GetNextVip2LevelInfo(CT_BYTE cbCurrVip2, CT_BYTE& cbNextVip2, CT_DWORD& dwNextVip2NeedRecharge)
{
	auto it = m_mapVip2Level.find(cbCurrVip2 + 1);
	if (it != m_mapVip2Level.end())
	{
		cbNextVip2 = it->first;
		dwNextVip2NeedRecharge = it->second;
	}
	else
	{
		cbNextVip2 = cbCurrVip2;
	}
}

CT_BYTE CLoginServerThread::GetProviceId(std::string& strProvice)
{
	auto it = m_mapProvince2Id.find(strProvice);
	if (it != m_mapProvince2Id.end())
	{
		return it->second;
	}

	return en_OTHER;
}


tagBenefitReward* CLoginServerThread::GetBenefitReward(CT_BYTE cbVip2)
{
	auto it = m_mapBenefitConfig.find(cbVip2);
	if (it != m_mapBenefitConfig.end())
	{
		return &it->second;
	}

	return NULL;
}

CT_BOOL CLoginServerThread::SplitPhoneNum(std::string& strPhoneFullNum, std::string& strCountryCode, std::string& strPhoneNum)
{
   // std::string strPhoneFullNum(pPhoneFullNum);
    auto it1 = strPhoneFullNum.find("+");

    //老用户号码
    if (it1 == std::string::npos)
    {
        return false;
    }

    auto it2 = strPhoneFullNum.find_last_of(")");
    if (it2 == std::string::npos)
    {
        return false;
    }

    if (it2 < it1)
        return false;

    if ((it2+1) == strPhoneFullNum.size())
        return false;

    strCountryCode = strPhoneFullNum.substr(it1+1, it2 - (it1+1));
    strPhoneNum =  strPhoneFullNum.substr(it2+1);

    return true;
}

bool CLoginServerThread::IsMobileNum(std::string& mobileNum)
{
    bool temp = false;
    //std::regex e("^1(3\\d|47|5([0-3]|[5-9])|8(0|2|[5-9]))\\d{8}$");
    //std::regex e("^(00|\\+)[1-9]{1}([0-9][\\s]*){9,16}$");
    std::regex e("^(\\+|00){0,2}(9[976]\\d|8[987530]\\d|6[987]\\d|5[90]\\d|42\\d|3[875]\\d|2[98654321]\\d|9[8543210]|8[6421]|6[6543210]|5[87654321]|4[987654310]|3[9643210]|2[70]|7|1)\\d{1,14}$");
    if (std::regex_match(mobileNum, e))
    {
        temp = true;
    }
    else
    {
        temp = false;
    }
    return temp;
}

