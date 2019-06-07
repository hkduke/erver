#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include <set>
#include <map>
#include "GlobalStruct.h"
#include "CMD_Inner.h"

typedef std::map<acl::aio_socket_stream*, CT_DWORD> MapClientSocketIp;

struct stGsServerUserBaseInfo
{
	CT_DWORD	dwUserID;
	CT_DWORD	dwServerID;
	acl::aio_socket_stream* pClientSocket;
	acl::aio_socket_stream* pGSSocket;
};

struct stClientUserHallInfo
{
	//CT_WORD		wGameID;		//当前登录的游戏
	CT_BYTE 		cbPlatformId;	//玩家属于哪个平台
	acl::aio_socket_stream* pClientSocket;
};

typedef std::map<CT_DWORD, stGsServerUserBaseInfo>	MapServerUser;
typedef std::map<acl::aio_socket_stream*, CT_DWORD> MapClientSock;
typedef std::map<CT_DWORD, stClientUserHallInfo>	MapHallUser;
typedef std::map<acl::aio_socket_stream*, acl::string> MapAndroidSock;
typedef std::map<CT_DWORD, CT_DWORD> MapGroupPRoom;

class CProxyThread : public CWorkThread
{
public:
	CProxyThread(void);
	~CProxyThread(void);

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
	//去总控服务器注册
	CT_VOID GoCenterServerRegister();
	//跟登录服务器绑定
	CT_VOID GoLoginServerBind(CNetConnector *pConnector);
	//跟游戏服务器绑定
	CT_VOID GoGameServerBind(CNetConnector *pConnector);
	//跟客服服务器绑定
	CT_VOID GoCustomerServerBind(CNetConnector *pConnector);

	CT_VOID InitServer();

private:
	CT_VOID OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnLoginServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

	CT_VOID OnProxyMainMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

	CT_VOID OnClientLoginMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientQueryMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientFrameMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientGameMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientGroupMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientHongBaoMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientTaskMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientRankMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientRechargeMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientCustomerMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientWealthGodComing(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientDuoBaoMsg(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnClientRedPacketMsg(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);\

private:
	//玩家的session是否存在.
	//CT_BOOL IsExistUserSession(CT_DWORD dwUserID, const CT_CHAR* pSession);
	//CT_BOOL	IsUserHasPrivateRoom(CT_DWORD dwUserID, CT_DWORD& dwRoomNum);
	//CT_BOOL	GetPrivateRoomInfo(CT_DWORD dwRoomNum, PrivateRoomInfo& roomInfo);
	//CT_WORD	GetUserLoginGameID(CT_DWORD dwUserID);
	//CT_VOID DelUserSession(CT_DWORD dwUserID);
	//CT_VOID DelProxyServerInfo(CT_DWORD dwUserID);
	//CT_VOID SetAccountInfoExpire(CT_DWORD dwUserID);
	//CT_VOID SetBenefitExpire(CT_DWORD dwUserID);
	//CT_VOID SetScoreInfoExpire(CT_DWORD dwUserID);
	//CT_VOID DelGameRoundCount(CT_DWORD dwUserID);
	//CT_VOID GetUserOnlineInfo(CT_DWORD dwUserID, std::map<acl::string, acl::string>& mapOnlineInfo);

	//CT_BOOL CheckUserCanEnterRoom(CT_DWORD dwUserID, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID, acl::aio_socket_stream* pSocket, CT_BOOL bCheckInSameGame);

private:
	CT_VOID CloseUserSockInThisGS(acl::aio_socket_stream* pGSSocket);

private:
	CT_VOID OnAccountRegister(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnAccountLoginOrRegister(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserSignIn(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserGetBenefit(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID	OnUserExchangeScore(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserPullMail(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserReadMail(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserDelMail(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindPromoter(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBindMobile(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserPayScoreByApply(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnAndroidConnect(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserBankOperate(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserGetVerificationCode(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID	OnUserResetPwd(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnUserModifyPersonalInfo(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnTranspondToLoginServer(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

private:
	//发送登录错误码
	CT_VOID SendLoginErrorCode(acl::aio_socket_stream* pSocket, CT_DWORD dwErrorCode);
	//发送游戏错误码
	CT_VOID SendGameErrorCode(acl::aio_socket_stream* pSocket, CT_WORD wMainID, CT_WORD wSubID, CT_DWORD dwErrorCode);
	 //发送玩家断开连接
	CT_VOID SendUserOffLine(acl::aio_socket_stream* pGSSock, CT_DWORD dwUserID);

	//更新人数
	CT_VOID UpdateUserCount(CT_WORD wSubID, CT_WORD wGameID);
	//注册用户
	CT_VOID RegisterUser(MSG_LS2P_UserLoginSucc* pUserLoginSucc);
	//反注册用户
	CT_VOID UnRegisterUser(CT_BYTE cbPlatformId, CT_DWORD dwUserID);

	//玩家下线
	CT_VOID GoLoginServerLogout(CT_DWORD dwUserID);

	//与中心服的心跳
  	inline CT_VOID SendHeartBeatToCenter();

private:
	MapClientSocketIp	m_mapClientSockIp;
	MapServerUser		m_mapGSUserInfo;
	MapClientSock		m_mapUserSock;
	MapHallUser			m_mapHallUser;
	MapAndroidSock		m_mapAndroidSock;
    MapGroupPRoom       m_mapGroupPRoom;

	//acl::redis_client	m_redisClient;
	//acl::redis			m_redis;
	CT_BOOL				m_bInitServer;

	static CT_DWORD		m_dwServerLinkCount;
};