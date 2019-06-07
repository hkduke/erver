#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "CMD_Customer.h"
#include <set>
#include <map>
#include "GlobalStruct.h"
#include "QueryRecordThread.h"

class CCustomerThread : public CWorkThread
{
public:
	CCustomerThread(void);
	~CCustomerThread(void);

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
	CT_VOID InitServer();

public:
	//客服管理主命令处理
	CT_VOID OnMsgCustomerMain(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen);
	//代理服务器主命令处理
	CT_VOID OnMsgProxyServerMain(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen);

	//子消息处理
	//客服发聊天
	CT_VOID OnSubCustomerChat(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen);

	//玩家访问
	CT_VOID OnSubUserVisit(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen);
	//玩家发聊天
	CT_VOID OnSubUserChat(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen);
	//玩家阅读消息
	CT_VOID OnSubUserReadMsg(const CT_VOID* pBuf, CT_DWORD wLen);

	//发送消息给客户端
	CT_VOID SendMsgToUser(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BYTE cbType);

public:
	//去总控服务器注册
	CT_VOID GoCenterServerRegister();

	inline acl::db_pool* GetChatdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_chatDBKey.c_str(), false, true);
		return pool;
	}

	acl::aio_socket_stream* GetUserProxySock(CT_DWORD dwUserID);
	CT_VOID SendUnReadMsgToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID);
	CT_DWORD GetWaitListUserByServerId(CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wKindID);
	CT_VOID ReadLast10Msg(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_BYTE cbType);

public:
	void PostMsgToCustomer(CT_DWORD dwUserID, CT_CHAR* pMsg, CT_CHAR* pIp, CT_BYTE cbType);
	std::string toHexString(const unsigned char* input, const int datasize);
    CT_VOID ReturnCommonToWeb(acl::aio_socket_stream* pSocket, std::string code, std::string strMessage);
	CT_VOID InsertQueryRecord(CT_DWORD dwUserID, acl::aio_socket_stream* pSocket, CT_BYTE cbMsgType);

private:
	//db操作
	acl::mysql_manager	m_dbManager;
	acl::string			m_chatDBKey;

	acl::redis_client	m_redisClient;
	acl::redis			m_redis;

	CT_BOOL				m_bInitServer;

	//std::map<CT_DWORD, std::vector<MSG_S2C_Chat_To_User>>			m_mapUserUnReadMsg;			//用户未读消息
	//std::map<CT_DWORD, std::vector<MSG_S2C_Chat_To_User>>			m_mapUserReportUnReadMsg;	//用户未读举报消息
	std::unique_ptr<CQueryRecordThread>	m_QueryRecordThreadPtr;
	Encrypt		m_encrypt;														//加密
};
