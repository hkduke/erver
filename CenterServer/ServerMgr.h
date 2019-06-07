#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Inner.h"
#include "CMD_Game.h"
#include "CMD_Customer.h"
#include <map>
#include <set>

typedef std::map<acl::aio_socket_stream*, CMD_ProxyServer_Info> MapProxyServer;
typedef std::map<acl::aio_socket_stream*, CMD_GameServer_Info> MapGameServer;
typedef std::map<CT_DWORD, CMD_GameServer_Info> MapGameServer2;
typedef std::map<acl::aio_socket_stream*, CMD_LoginServer_Info> MapLoginServer;
typedef std::map<acl::aio_socket_stream*, CMD_DipServer_Info> MapDipServer;
typedef std::map<acl::aio_socket_stream*, CMD_CustomerServer_Info> MapCustomerServer;

typedef std::map<CT_DWORD, std::map<CT_WORD, CT_WORD>> MapPRoomNeedGem;
typedef std::map<CT_DWORD, std::set<CT_WORD>> MapPRoomUserCount;

//--<platformId, <--gameid*100+kindid*10+roomkind, userCount>>
//typedef std::map<CT_BYTE, std::map<CT_DWORD, CT_DWORD>> MapGameUserCount;

class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr();
	~CServerMgr();

public:
	//添加代理服
	CT_BOOL AddProxyServerInfo(acl::aio_socket_stream* pSocket, const CMD_ProxyServer_Info* pProxyInfo);
	//查找代理服
	acl::aio_socket_stream* FindProxyServer(CT_DWORD dwServerID);
	CMD_ProxyServer_Info* FindProxyServerInfo(acl::aio_socket_stream* pSocket);
    //获取当时的ps合集
    const MapProxyServer* GetProxyServerMap();

	//添加游戏服务器
	CT_BOOL AddGameServerInfo(acl::aio_socket_stream* pSocket, const CMD_GameServer_Info* pGameInfo);
	//查找游戏服
	acl::aio_socket_stream* FindGameServer(CT_DWORD dwServerID);
	//查找游戏服
	acl::aio_socket_stream* FindGameServer(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);
	//查找游戏服
	CMD_GameServer_Info* FindGameServerEx(CT_DWORD dwServerID);
	//查找游戏服
	CMD_GameServer_Info* FindGameServer2(CT_DWORD dwServerID);
	//更新游戏服的人数
	CT_VOID UpdateGameServerUser(acl::aio_socket_stream* pSocket, const CMD_Update_GS_User* pGameInfo);
	//计算某个游戏类型的在线人数
	CT_VOID CalGameServerUserCount(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID, CT_DWORD& dwKindUserCount, CT_DWORD& dwRoomKindUserCount);
	//获取游戏服务器名称
	std::string GetGameServerName(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);
	//设置游戏服务器的状态
	CT_VOID SetGameServerState(CT_DWORD dwServerID, CT_BYTE cbState);
	//解散所有游戏的私人房间
	CT_VOID DismissServerPRoom(CT_DWORD dwServerID, CT_DWORD dwRoomNum);
	//获取当时的gs合集
	const MapGameServer* GetGameServerMap();
	//发送消息给所有游戏服务器
	CT_VOID SendMsgToAllGameServer(CT_WORD mainId, CT_WORD subId, CT_VOID* pData, CT_DWORD dwLen);

	//增加平台游戏人数
	//CT_VOID IncreaseGameUserCount(CT_BYTE cbPlatformId, CMD_Update_GS_User* pUpdateUserCount);
	//减少平台游戏人数
	//CT_VOID DecreaseGameUserCount(CT_BYTE cbPlatformId, CMD_Update_GS_User* pUpdateUserCount);
	//获取某个平台的游戏人数
	//CT_DWORD GetGameUserCount(CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);

	//添加登录服务器
	CT_BOOL AddLoginServerInfo(acl::aio_socket_stream* pSocket, const CMD_LoginServer_Info* pLoginInfo);
	//查找登录服
	acl::aio_socket_stream* FindLoginServerInfo(CT_DWORD dwServerID);
	//广播消息给登录服
	CT_VOID SendMsgToAllLoginServer(CT_WORD mainId, CT_WORD subId, const CT_VOID* pData, CT_DWORD dwLen);

	//添加DIP服务器
	CT_BOOL AddDipServerInfo(acl::aio_socket_stream* pSocket, const CMD_DipServer_Info* pDipInfo);
	//查找DIP服
	acl::aio_socket_stream* FindDipServer(CT_DWORD dwServerID);

	//添加客服服务器
	CT_BOOL AddCustomerServerInfo(acl::aio_socket_stream* pSocket, const CMD_CustomerServer_Info* pCustomer);
	//查找登录服
	acl::aio_socket_stream* FindCustomerServerInfo(CT_DWORD dwServerID);

	//查找一个合适的游戏服务器(查找人数最多，而且还没有达到最大人数的服务器)
	//房卡类型适用
	CMD_GameServer_Info* FindSuitTableGameServer(CT_WORD wGameID, CT_WORD wKindID);
	//金币房间适用
	CMD_GameServer_Info* FindSuitTableGameServer(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);

	//检查是否参数不对
	CT_BOOL CheckCreatePRoomParam(CT_DWORD dwServerID, MSG_PS_CreatePrivateRoom* pPrivateRoom);
	//查找某个游戏服创建私人房间需要的钻石
	CT_WORD GetCreatePRoomNeedGem(CT_DWORD dwServerID, CT_WORD wPlayCount);

	//发送游戏服务器列表
	CT_BOOL SendGameServerList(acl::aio_socket_stream* pSocket, CT_WORD mainId, CT_WORD subId, CT_WORD finishSubId);
	//发送登录服务器列表
	CT_BOOL SendLoginServerList(acl::aio_socket_stream* pSocket);
	//发送客服服务器列表
	CT_BOOL SendCustomerServerList(acl::aio_socket_stream* pSocket);

	//广播游戏服务器的状态给proxy
	CT_VOID BroadcastGameServerStateToProxy(CT_DWORD dwServerID, CT_BYTE cbState);

	//发送游戏服务器的状态给gameserver
	CT_VOID SendGameServerStateToGame(CT_DWORD dwServerID, CT_BYTE cbState);

	//注销服务器
	CT_VOID DeleteServer(acl::aio_socket_stream* pSocket);

private:
	MapProxyServer		m_mapProxyServer;
	MapGameServer		m_mapGameServer;
	MapGameServer2		m_mapGameServer2;
	MapLoginServer		m_mapLoginServer;
	MapDipServer		m_mapDipServer;
	MapCustomerServer   m_mapCustomerServer;

	MapPRoomNeedGem		m_mapPRoomNeedGem;
	MapPRoomUserCount	m_mapPRoomUserCount;

	//MapGameUserCount	m_mapGameUserCount;		//这是个冗余数据，主要只于后台统计
};