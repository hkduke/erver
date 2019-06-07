#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CDBConnector.h"
#include "CMD_Plaza.h"
#include "CTType.h"
#include "GlobalStruct.h"
#include "NetEncrypt.h"
#include "IpParse.h"
#include "LoginRecordThread.h"
#include "VerificationCodeThread.h"
#include "ExchangePostThread.h"
#include <memory>

class CLoginServerThread : public CWorkThread
{
public:
	CLoginServerThread(void);
	~CLoginServerThread(void);

public:
	struct stSignInData
	{
		CT_WORD			wSignInDays;						// 连续签到天数
		CT_BOOL			bNewDate;							// 是否有新签到
		CT_BOOL			bFirstSignIn;						// 是否第一次签到
		
		stSignInData()
		{
			Reset();
		}
		void Reset()
		{
			wSignInDays = 0;
			bNewDate = false;
			bFirstSignIn = false;
		}
	};

	struct stSignInReward
	{
		CT_WORD			wRewardGem;
		CT_DWORD		dwRewardScore;
	};

	struct stVipReward
	{
		CT_DWORD		dwSignInRewardScore;
	};

	struct stGameRoundInfo
	{
		CT_WORD wGameID;
		CT_WORD wKindID;
		CT_DWORD dwGameRoundCount;
	};

	struct stGameVersion
	{
		CT_WORD wMainVer;
		CT_WORD wSubVer;
		CT_WORD wBuildVer;
	};

	struct stChannelPresentScore
    {
        CT_DWORD dwRegisterPresentScore;
        CT_DWORD dwBindMobilePresentScore;
    };

	struct stUserPlayData
    {
		stUserPlayData()
		//:dwGameIndex(0)
		:dwPlayCount(0)
		,llWinScore(0)
		{ }

		//CT_DWORD 	dwGameIndex;		//gameid*10000+kindid*100+roomkind
	    CT_DWORD  	dwPlayCount;		//游戏局数
	    CT_LONGLONG llWinScore;			//输赢分
    };

	enum enMailType
	{
		en_Mail_Delete_AfterRead	= 1,			//阅后即焚
		en_Mail_Delete_Expire		= 2,			//过期销毁
	};

	enum enMailState
	{
		en_Mail_State_UnRead		= 1,			//未读
		en_Mail_State_Read			= 2,			//已读取
		en_Mail_State_Login_Popup	= 3,			//登录弹出
	};

	enum enPresentMode
    {
        en_Register = 1,
        en_BindMobile =2,
    };

public:
	//网络连接
	virtual CT_VOID OnTCPSocketLink(CNetConnector* pConnector);
	//网络连接关闭
	virtual CT_VOID OnTCPSocketShut(CNetConnector* pConnector);
	//网络应答
	virtual CT_VOID OnTCPNetworkBind(acl::aio_socket_stream* pSocket);
	//网络关闭
	virtual CT_VOID OnTCPNetworkShut(acl::aio_socket_stream* pSocket);
	//网络数据到来
	virtual CT_VOID OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen);
	//网络定时器
	virtual CT_VOID OnTimer(CT_DWORD dwTimerID);

	//打开
	virtual CT_VOID Open();

private:
	CT_VOID OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

	CT_VOID OnUserLoginMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

private:
	//去总控服务器注册
	CT_VOID GoCenterServerRegister();

	inline CT_VOID AcrossTheDayEvent();

	//用户登录相关
private:
	CT_VOID OnAccountsLogin(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnAccountsRegister(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnAccountsLoginOrRegister(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserSignIn(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserQueryBenefit(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserGetBenefit(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserExchangeScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindPromoter(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindMobile(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserPayScoreByApple(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserPullMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserReadMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserDelMail(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserSetBankPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserModifyBankPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserSaveBankScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserTakeBankScore(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserGetVerifyCode(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserResetPwd(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserModifyPersonalInfo(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindAliPay(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindBankCard(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserExchangeScoreToRmb(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserExchangeScoreRecord(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserLogout(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnUserUnRedExchange(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserReportUser(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserQueryRechargeRecord(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	
	//与中心服通讯的数据
private:
	CT_VOID OnUpdateShowExchangeCond(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnUpdateChannelPresentScore(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnUpdateExchangeInfo(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnUpdateBenefitConfig(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnUpdateSMSPid(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID GoCenterAddGroupUser(UserAccountsBaseData& accountData, CT_DWORD dwParentUserID);

private:
	CT_BYTE GoDBAccountLoginOrRegister(acl::db_handle* db, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData);
	//CT_BYTE GoDBAccountLoginOrRegister(MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData);
	CT_BYTE GoDBAccountRegister(acl::db_handle* db, MSG_CS_LoginDataEx* pRegister, UserAccountsBaseData& accountData);
	CT_BYTE GoDBAccountLogin(acl::db_handle* db, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData);
	CT_BYTE GoDBBindMobile(acl::db_handle* db, MSG_CS_Bind_Mobile* pBindAccount, std::string& oldAccount, acl::string& newNickName, CT_BYTE& cbPlatformId);
	CT_BYTE GoDBCheckMobileBind(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum);
	CT_BYTE GoDBResetPwd(MSG_CS_Reset_PwdEx* pReset);
	CT_BYTE GoDBModifyPersonalInfo(MSG_CS_Modify_PersonalInfo* pModifyInfo);
	CT_BYTE	GoDBBindExchangeAliPay(MSG_CS_Bind_AliPay* pBindAliPay);
	CT_BYTE	GoDBBindExchangeBankCard(MSG_CS_Bind_BankCard* pBankCard);
	CT_DWORD GetNewUserIDFromDB();
	inline CT_WORD	GetIpRegisterCount(CT_DWORD dwIp);
	inline CT_VOID	IncreaseRegisterCount(CT_DWORD dwIp);

	CT_VOID SendLoginFail(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_INT32 iErrorCode);
	CT_VOID SendLoginSuccess(acl::aio_socket_stream* pSocket, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData);
	inline CT_VOID SendMsg(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen);
	inline CT_VOID SendMsgByUserID(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen);
	inline std::string toHexString(const unsigned char* input, const int datasize);

	inline CT_VOID SendProxyServerLoginSucc(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, UserAccountsBaseData& accountData, CT_WORD wGameID);
	inline CT_VOID SendKickUserMsg(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, CT_DWORD dwUserID, CT_BOOL bSendToClient);
	CT_VOID SendGameItem(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID);
	CT_VOID SendGameRoomKind(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID);
	CT_VOID SendGamePrivateRoomInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwGameID);
	CT_VOID SendSelfPrivateRoomInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID);
	inline CT_VOID SendUserOnlineInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, std::map<acl::string, acl::string>& mapOnlineInfo);
	CT_VOID SendUserSignInInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID);
	CT_VOID SendUserWelfareInfo(acl::aio_socket_stream* pSocket, CT_INT64 dwClientValue, CT_DWORD dwUserID);	//签到 救济金等
	CT_VOID SendUserScoreInfo(acl::aio_socket_stream* pSocket, UserAccountsBaseData& accountData, CT_UINT64 dwClientValue);
	inline CT_VOID SendUserMail(acl::aio_socket_stream* pSocket, CT_UINT64 dwClientValue, CT_DWORD dwUserID);
	inline CT_VOID SendUserGameVersion(acl::aio_socket_stream* pSocket, CT_UINT64 uClinetValue);
	inline CT_VOID SendUserBenefit(acl::aio_socket_stream* pSocket, UserAccountsBaseData& accountData, CT_UINT64 uClinetValue);
	inline CT_VOID SendUserAllSucMsg(acl::aio_socket_stream* pSocket, MSG_CS_LoginDataEx* pLoginData, UserAccountsBaseData& accountData, std::map<acl::string, acl::string>& mapOnlineInfo);

private:
	CT_VOID AddUserGemAndScore(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore, CT_BOOL bUpdateToClient = false);
	inline CT_VOID AddUserBankScore(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llAddBankScore);
	CT_VOID UpdateUserGemToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_DWORD dwNewGem);
	CT_VOID UpdateUserScoreToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_LONGLONG llNewScore);
	CT_VOID UpdateUserBankScoreToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llNewBankScore);

private:
	inline CT_VOID SetUserSession(CT_DWORD dwUserID, const std::string& strUUID);
	inline CT_VOID SetUserAccountInfo(const CT_CHAR* pAccountName, const UserAccountsBaseData& accountData);
	inline CT_VOID SetAccountUserID(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, CT_DWORD dwUserID);
	inline CT_VOID SetUserBindMobileInfo(CT_DWORD dwUserID, const CT_CHAR* pNewAccountName, const CT_CHAR* pNickName, const CT_CHAR* pPassword);
	inline CT_VOID SetUserPwd(const CT_CHAR* pNewAccountName, const CT_CHAR* pPassword, CT_DWORD& dwUserID, CT_BYTE cbPlatformId);
	inline CT_VOID SetUserBankPwd(CT_DWORD dwUserID, const CT_CHAR* pPassword);
	inline CT_VOID RenameAccountUserID(const CT_CHAR* pOldAccountName, const CT_CHAR* pNewAccountName, CT_BYTE cbPlatformId);
	inline CT_VOID SetUserPersonalInfo(MSG_CS_Modify_PersonalInfo* pPersonalInfo);
	inline CT_VOID SetProxyServerInfo(acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue, CT_DWORD dwUserID, const CT_CHAR* szMachineSerial);
	inline CT_VOID SetUserGameRound(CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo);
	inline CT_VOID SetUserSignIn(CT_DWORD dwUserID);
	inline CT_VOID SetUserBenefitInfo(CT_DWORD dwUserID);
	inline CT_VOID SetUserPlayData(CT_DWORD dwUserID,
								   std::map<CT_DWORD, std::unique_ptr<stUserPlayData>> &mapUserPlayData);
	inline CT_VOID SetUserScoreInfo(CT_DWORD dwUserID);
	inline CT_VOID SetUserBindPromoterID(CT_DWORD dwUserID, CT_DWORD dwPromoterID);
	inline CT_VOID SetUserTaskInfo(CT_DWORD dwUserID);
	inline CT_VOID SetUserEarnScore(CT_DWORD dwUserID, CT_LONGLONG llEarnScore);
	inline CT_VOID SetUserAliPay(MSG_CS_Bind_AliPay* pBindAliPay);
	inline CT_VOID SetUserBankCard(MSG_CS_Bind_BankCard* pBankCard);
	inline CT_VOID SetMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum, CT_DWORD dwVerifyCode);
	inline CT_VOID SetUserLocationAndIp(CT_DWORD dwUserID, const CT_CHAR* pLocation, const CT_CHAR* pIp);
	inline CT_VOID SetUserTodayRankInfo(CT_DWORD dwUserID, CT_DWORD dwTodayOnline, CT_WORD wTodayTaskFinish, CT_DWORD dwTodayTaskReward);
	inline CT_VOID SetUserGoodCardInfo(UserAccountsBaseData& accountData);

	CT_VOID SetRedisKeyPersist(acl::string key);

	CT_BOOL GetAccountInfo(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, UserAccountsBaseData& accountData);
	CT_LONGLONG GetUserScore(CT_DWORD dwUserID);
	CT_LONGLONG GetUserBankScore(CT_DWORD dwUserID);
	CT_DWORD GetUserGem(CT_DWORD dwUserID);
	CT_BYTE	 GetUserVipLevel(CT_DWORD dwUserID);
	CT_BYTE  GetUserVip2Level(CT_DWORD dwUserID);
	CT_WORD GetUserChannelID(CT_DWORD dwUserID);
	CT_BOOL CheckUserOtherGameOnline(CT_DWORD dwUserID, CT_DWORD wGameID, std::map<acl::string, acl::string>& mapOnlineInfo);
	CT_BOOL CheckUserInGameOnline(CT_DWORD dwUserID);
	CT_BOOL IsExistUserSession(CT_DWORD dwUserID, const CT_CHAR* pSession);
	CT_VOID AddUserGemAndScoreToCache(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore, CT_BOOL bUpdateToClient);
	CT_VOID AddUserBankScoreToCache(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_LONGLONG llAddBankScore);
	CT_VOID AddUserGemAndScoreToCenterServer(CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore);
	CT_VOID AddBankScoreToCenterServer(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore);

	CT_BOOL	GetUserBenefitInfo(CT_DWORD dwUserID);
	CT_BOOL GetUserScoreInfo(CT_DWORD dwUserID);
	CT_DWORD GetUserBindPromoterID(CT_DWORD dwUserID);
	CT_BOOL GetUserBankPwd(CT_DWORD dwUserID, std::string& strBankPwd);
	CT_BOOL GetMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum, CT_DWORD& dwVerifyCode);
	CT_BYTE GetUserPlatformId(CT_DWORD dwUserID);
	CT_VOID DeleteMobileVerifyCode(CT_BYTE cbPlatformId, CT_CHAR* pMobileNum);
	CT_VOID GetUserAliPayAccount(CT_DWORD dwUserID, acl::string& strAliPay);
	CT_VOID GetUserBankCard(CT_DWORD dwUserID, acl::string& strBankCard);
	CT_VOID GetUserMobileNum(CT_DWORD dwUserID, acl::string& strMobileNum);


	//玩家登出相关redis操作
	CT_VOID DelUserSession(CT_DWORD dwUserID);
	CT_BOOL DelProxyServerInfo(CT_DWORD dwUserID, CT_DWORD dwProxyServerID);
	CT_VOID SetAccountInfoExpire(CT_DWORD dwUserID);
	CT_VOID SetBenefitExpire(CT_DWORD dwUserID);
	CT_VOID SetScoreInfoExpire(CT_DWORD dwUserID);
	CT_VOID SetGameRoundCountExpire(CT_DWORD dwUserID);
	CT_VOID SetTaskExpire(CT_DWORD dwUserID);
	CT_VOID	SetEarnScoreExpire(CT_DWORD dwUserID);
	CT_VOID SetAccountRelatedPersist(CT_BYTE cbPlatformId, CT_CHAR* pAccountName, CT_DWORD dwUserID); //accountName在未绑定手机时，是手机序列码，绑定手机后是手机号

	//DB操作
private:
	inline  CT_VOID QuerySignInInfo(CT_DWORD dwUserID);
	inline  CT_VOID QuerySignInInfoIndb(acl::db_handle* db, CT_DWORD dwUserID, stSignInData& signInData);
	inline  CT_VOID UpdateSignInRecord(acl::db_handle* db, CT_DWORD dwUserID/*, CT_DWORD dwRewardGem, CT_DWORD dwRewardScore*/);
	inline  CT_VOID QueryBenefitInfo(CT_DWORD dwUserID);
	inline  CT_VOID QueryBenefitInfoIndb(acl::db_handle* db, CT_DWORD dwUserID, tagBenefitData& benefitData);
	inline  CT_VOID InsertBenefitRecord(acl::db_handle* db, CT_DWORD dwUserID, CT_LONGLONG llRewardScore);
	inline  CT_VOID QueryGamePlayData(CT_DWORD dwUserID);
	inline  CT_VOID AddUserGemAndScoreTodb(acl::db_handle* db, CT_DWORD dwUserID, CT_INT32 nAddGem, CT_LONGLONG llAddScore);
	inline  CT_VOID InsertScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBank, CT_LONGLONG llSourceScore, CT_LONGLONG llAddBank, CT_LONGLONG llAddScore, CT_BYTE cbType, CT_DWORD dwMailID = 0);
	inline  CT_VOID AddUserBankScoreTodb(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore);

	inline  CT_VOID QueryUserGameRound(CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo);
	inline  CT_VOID QueryUserGameRound(acl::db_handle* db, CT_DWORD dwUserID, std::vector<stGameRoundInfo>& vecGameRoundInfo);

	inline  CT_VOID QueryUserMail(CT_DWORD dwUserID, CT_DWORD dwChannelID,  std::vector<MSG_SC_Mail_Info>& vecUserMail);
	inline  CT_VOID QueryUserMailIndb(acl::db_handle* db, CT_DWORD dwUserID,  CT_DWORD dwChannelID, std::vector<MSG_SC_Mail_Info>& vecUserMail);
	inline  CT_BOOL QueryUserOneMailIndb(acl::db_handle* db, CT_DWORD dwMailID, MSG_SC_Mail_InfoEx& mailInfo);
	inline  CT_VOID SendMail(tagUserMail& mail);

	inline  CT_VOID QueryScoreInfo(CT_DWORD dwUserID);
	inline  CT_VOID QueryScoreInfo(acl::db_handle* db, CT_DWORD dwUserID);
	inline  CT_VOID QueryTaskInfo(CT_DWORD dwUserID);
	inline  CT_VOID	QueryTaskInfo(acl::db_handle* db, CT_DWORD dwUserID);
	inline  CT_VOID QueryGoodCardInfo(UserAccountsBaseData& userAccountsBaseData);
	inline 	CT_VOID UpdateUserGoodCardInfo(UserAccountsBaseData& userAccountsBaseData);

	inline  CT_VOID QueryUserEarnScore(CT_DWORD dwUserID);
	inline  CT_VOID QueryUserTodayRankInfo(CT_DWORD dwUserID);
	inline  CT_BYTE BindUserPromoterID(acl::db_handle* db, CT_DWORD dwUserID, CT_DWORD dwPromoterID);

	CT_VOID UpdateUserBankPwd(CT_DWORD dwUserID, CT_CHAR* pBankPwd);

	//插入兑换订单
	CT_BOOL CheckAliPayOrBankCardLimited(const CT_CHAR* pPayAccount);
	CT_VOID InsertExchangeOrder(MSG_CS_Exchange_Score_To_Rmb* pExchange);
	CT_VOID QueryExchangeOrderRecord(CT_DWORD dwUserID, std::vector<MSG_SC_Exchange_Rmb_Record>& vecRecord);
	CT_WORD GetTodayExchangeOrderCount(CT_DWORD dwUserID, CT_BYTE cbExchangeChannelID);
	inline CT_VOID QuyerRechargeOrderRecord(CT_DWORD dwUserID, std::vector<MSG_SC_RechargeRecord>& vecRecord);

	//插入修改记录
	CT_VOID InsertModifyRecord(CT_DWORD dwUserID, CT_BYTE cbType, CT_CHAR* pContent);

	//内部函数
private:
	inline CT_VOID GetNextVip2LevelInfo(CT_BYTE cbCurrVip2, CT_BYTE& cbNextVip2, CT_DWORD& dwNextVip2NeedRecharge);
	inline CT_BYTE GetProviceId(std::string& strProvice);
	inline tagBenefitReward* GetBenefitReward(CT_BYTE cbVip2);
	inline CT_BOOL SplitPhoneNum(std::string& strPhoneFullNum, std::string& strCountryCode, std::string& strPhoneNum);
    inline CT_BOOL IsMobileNum(std::string& mobileNum);

private:
	CT_VOID InitServer();

	//连接账户DB
	//CT_BOOL ConnectAccountDB();
	//账户DB错误
	//CT_VOID OnAccountDBError();

	//连接游戏DB
	//CT_BOOL ConnectGamePHZDB();
	//连接DB错误
	//CT_VOID OnGamePHZDBError();

	//链接账户DB
	//CT_BOOL ConnectPlatformDB();
	//账户DB错误
	//CT_VOID OnPlatformDBError();

	//执行无任何操作存储过程,防止mysql连接断开.
	//CT_VOID ExeNoThingProc();
	
	CT_VOID LoadGameKind(acl::db_handle* db);
	CT_VOID LoadGameRoomKind(acl::db_handle* db);
	CT_VOID LoadSignInConfig(acl::db_handle* db);
	CT_VOID LoadVipConfig(acl::db_handle* db);
	CT_VOID LoadVip2Config(acl::db_handle* db);
	CT_VOID	LoadBenefitConfig(acl::db_handle* db);
	CT_VOID LoadExchangeScoreConfig(acl::db_handle* db);
	CT_VOID LoadGameVersion(acl::db_handle* db);
	CT_VOID LoadRegisterReward(acl::db_handle* db);
	CT_VOID LoadSystemStatus(acl::db_handle* db);
	CT_VOID LoadChannelShowExchange(acl::db_handle* db);
	CT_VOID LoadExchangeInfo(acl::db_handle* db);
	CT_VOID LoadChannelPresentScore(acl::db_handle* db);
	CT_VOID LoadChannelSendRegisterMail(acl::db_handle* db);
	//CT_VOID	InsertUserID();

public:
	CT_VOID GetGameVersion(std::string strVersion, stGameVersion& version);

	inline acl::db_pool* GetAccountdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_accountDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetRecorddbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_recordDBKey.c_str(), false, true); 
		return pool;
	}
	inline acl::db_pool* GetPlatformdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_platformDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetGamedbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_gameDBKey.c_str(), false, true);
		return pool;
	}

private:
	//登录日志记录
	inline CT_VOID InsertLoginRecord(CT_DWORD dwUserID, MSG_CS_LoginDataEx* pLogin);
	//查找是否能显示
	inline CT_BYTE GetChannelShowExchange(CT_DWORD dwChannelID);
	//查找多少钱能显示兑换按钮
	inline CT_DWORD GetChannelShowExchangeRecharge(CT_DWORD dwChannelID);
	//查找赠送金币
    inline CT_DWORD GetChannelPresentScore(CT_WORD wChannelID, enPresentMode cbMode);
    //登陆发送注册邮件
	inline CT_BOOL IsChannelSendRegisterMail(CT_WORD wChannelID);


private:
	//帐号数据库
	//CCDBConnector		m_accountDBCon;
	//CCDBConnector		m_platformDBCon;
	//CCDBConnector		m_gamePHZDBCon;

	//redis
	acl::redis_client	m_redisClient;
	acl::redis			m_redis;

	//redis (for register ip)
	acl::redis_client	m_redisClient2;
	acl::redis			m_redis2;
	
private:
	//签到
	std::map<CT_INT32, stSignInReward>			m_mapsignInConfig;
	stSignInData								m_signInData;

	//VIP额外奖励
	std::map<CT_DWORD, stVipReward>				m_mapVipReward;

	//VIP2等级
	std::map<CT_BYTE, CT_DWORD>					m_mapVip2Level;

	//渠道是否显示兑换按钮
	std::map<CT_WORD, tagChannelShowExchangeCond> m_mapChannelShowExchange;
	//渠道注册赠送金币
    std::map<CT_DWORD, stChannelPresentScore>    m_mapChannelPresentScore;
    //渠道是否注册发送邮件
	std::map<CT_WORD, CT_BYTE>					m_mapChannelRegisterMail;

	//救济金
	std::map<CT_BYTE, tagBenefitReward>			m_mapBenefitConfig;
	//tagBenefitReward							m_benefitCofig;
	tagBenefitData								m_benefitData;
	
	//钻石兑换配置
	std::map<CT_WORD, tagExchangeScoreConfig>	m_mapExchageScoreCfg;

	//玩家局数数据
	//std::vector<stGameRoundInfo>				m_vecGameRoundInfo;

	//玩家邮件
	std::vector<MSG_SC_Mail_Info>				m_vecUserMail;
	//版本号
	std::map<CT_WORD,  CT_WORD>					m_mapGameVersion;

	//金币相关信息
	UserScoreData								m_scoreData;
	//任务信息
	std::vector<tagUserTask>					m_vecTask;

	//各省ID对照表
	std::map<std::string, CT_BYTE>				m_mapProvince2Id;

private:
	acl::mysql_manager	m_dbManager;
	acl::string			m_accountDBKey;
	acl::string			m_recordDBKey;
	acl::string			m_platformDBKey;
	acl::string			m_gameDBKey;
	std::unique_ptr<acl::mysql_pool> m_accountDBPool;

	CIpFinder			m_pIpFinder;

	CT_INT32			m_iRegisterPresentGem;		            //赠送钻石数量
	CT_INT32			m_iRegisterPresentScore;	            //赠送积分数量
	CT_INT32			m_iBindMobileScore;			            //绑定手机送积分
	//CT_BYTE				m_cbSignInStatus;			        //签到开关（0关闭1开启）
	//CT_BYTE				m_cbBenefitStatus;			        //救济金开关（0关闭1开启）
	CT_BYTE				m_cbScoreMode;				            //是否开金币场模式
	CT_BYTE				m_cbPrivateMode;			            //是否开私人房模式
	CT_BYTE				m_cbOpenApplePay;			            //是否打开苹果充值
	CT_WORD				m_wIpRegisterLimite;		            //Ip注册限制次数
	
	std::vector<tagExchangeChannelAmount> m_vecExchangeInfo;    //兑换信息

	CT_BOOL				m_bInitServer;
	Encrypt				m_encrypt;
	acl::md5			m_MD5;

	CT_INT64			m_iTodayTime;				            //今天某个时间戳
	
	std::unique_ptr<CLoginRecordThread>			m_LoginRecordThreadPtr;
	//std::unique_ptr<CVerificationCodeThread>	m_VerificationCodeThreadPtr;
	//std::unique_ptr<CExchangePostThread>		m_ExchangePostThreadPtr;
};