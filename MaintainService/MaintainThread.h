#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include <set>
#include <map>
#include "GlobalStruct.h"
#include "CMD_Inner.h"

class CMaintainThread : public CWorkThread
{
public:
	CMaintainThread(void);
	~CMaintainThread(void);

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
	
private:
	CT_VOID OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

private:
	CT_VOID AddGameServerList(const CMD_GameServer_Info* pGameServer);
	CT_VOID	SelectOperation();

	typedef std::map<CT_DWORD, CMD_GameServer_Info>	MapGameServerInfo;
	MapGameServerInfo	m_mapGameServer;
};	