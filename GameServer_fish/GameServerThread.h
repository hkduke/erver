#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "GlobalStruct.h"
#include "CDBConnector.h"
#include "CMD_Game.h"
#include "ServerUserItem.h"
#include "WXGroupMgr.h"
#include "GameTableManager.h"

//游戏玩家基础信息
typedef struct strGameUserBaseInfo
{
	CT_DWORD	dwRoomID;			// 房间ID
	CT_DWORD	dwTableID;			// 桌子ID
	CT_DWORD	dwChairID;			// 椅子ID
}GameUserBaseInfo;

typedef std::map<CT_DWORD, GameUserBaseInfo>	GameUserInfoList;
typedef GameUserInfoList::iterator				IterGameUserInfoList;

class CGameServerThread : public CWorkThread
{
public:
	CGameServerThread(void);
	~CGameServerThread(void);

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
	//中心服发来的框架消息
	void OnCenterServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	//客户端发来的框架消息
	void OnProxyServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	//客户端发来的框架消息
	void OnClientFrameMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	//客户端发来的游戏消息
	void OnClientGameMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	//DB服务器返回的消息
	void OnDBServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	//第三方游戏服务器(捕鱼)的消息
	void OnFishGameServerLogonMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	void OnFishGameServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	void OnFishGameClientMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);
	void OnFishGameWriteScoreMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize);

private:
	//更新金币
	CT_VOID AddUserScore(CServerUserItem* pUserItem, CT_LONGLONG llScore);
	//更新玩家金币
	CT_VOID UpdateUserScoreToDB(CT_DWORD dwUserID, CT_LONGLONG llAddScore);
	CT_VOID UpdateUserScoreToCenterServer(CT_DWORD dwUserID, CT_LONGLONG llSourceScore, CT_LONGLONG llAddScore, enScoreChangeType enType);
	CT_VOID BroadcastUserForUser(CServerUserItem* pFromUser, CServerUserItem* pToUser);
	CT_BOOL RecordRawInfo(RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CServerUserItem* pUserItem);
	CT_VOID TickOffLineUser();

public:
	//响应玩家准备
	CT_VOID OnUserReadyMsg(CT_DWORD dwUserID);
	//玩家离线
	CT_VOID OnUserOffLine(CT_DWORD dwUserID);
	//玩家离开
	CT_VOID OnUserLeft(CT_DWORD dwUserID, CT_BOOL bSendStateMyself = true, CT_BOOL bForceLeave = false);
	//玩家申请解散房间
	CT_VOID OnUserApplyDismissPRoom(CT_DWORD dwUserID);
	//玩家操作解散房间
	CT_VOID OnUserOperateDismissPRoom(CT_DWORD dwUserID, CT_BYTE cbOperateCode);
	//玩家语音
	CT_VOID OnUserVoiceChat(CT_DWORD dwUserID, CT_CHAR* pChatUrl);
	//文字聊天
	CT_VOID OnUserTextChat(CT_DWORD dwUserID, CT_BYTE cbType, CT_BYTE cbIndex);
	//房主拉取私人房信息
	CT_VOID OnPRoomOwnerPullApply(CT_DWORD dwUserID);
	//房主处理申请入私人房信息
	CT_VOID OnPRoomOwnerOperateApply(CT_DWORD dwOwnerID, CT_DWORD dwUserID, CT_BYTE cbOperateCode);
    //查询所有桌子信息
    acl::string OnQueryTableInfo(CT_BYTE cbPlatformId);
private:
	CT_VOID InitServer();
	//去中心服务器注册
	CT_VOID GoCenterServerRegister();
	//DB读取游戏配置信息
	CT_VOID LoadGameKind();
	CT_VOID LoadGameRoomKind();
	CT_VOID LoadGameRoundReward();
	CT_VOID	LoadAndroidParam();
	CT_VOID	LoadBenefitConfig();
	CT_VOID InsertGameServerRoomInfo();
	CT_BOOL ConnectPlatformDB();

	//功能函数
private:
	//redis相关
	inline CT_BOOL GetUserBaseInfo(CT_DWORD dwUserID, GS_UserBaseData& userBaseData);
	inline CT_BOOL GetUserPRoomInfo(CT_DWORD dwUserID, GS_UserPRoomData& userPRoomData);
	inline CT_VOID DelPRoomInfo(CT_DWORD dwRoomNum);
	inline CT_VOID	AddUserGem(CT_DWORD dwUserID, int iAddGem);
	inline CT_BOOL CheckHasThisPRoom(CT_DWORD dwRoomNum);
	inline CT_VOID GetUserOnlineInfo(CT_DWORD dwUserID, std::map<acl::string, acl::string>& mapOnlineInfo);
	inline CT_VOID SetUserOnlineInfo(CT_DWORD dwUserID);
	inline CT_VOID DelUserOnlineInfo(CT_DWORD dwUserID);
	inline CT_BOOL CheckUserCanEnterRoom(CT_DWORD dwUserID, acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue);
	inline CT_VOID AddEarnScoreInfo(CServerUserItem* pUserItem, CT_LONGLONG llAddEarnScore);

	//操作相关
	CT_VOID AddWaitList(CT_DWORD dwUserID, CT_UINT64 uClientAddr, acl::aio_socket_stream* pProxySock);
    //更新服务器等待玩家数量
    CT_VOID UpdateServerIDWaitListCount(const CT_DWORD &dwWaitListCount);
	CT_VOID DistributeTable();

public:
	CT_VOID	OnUserEnterPrivateRoom(acl::aio_socket_stream* pSocket, MSG_PG_EnterPrivateRoom* pPrivateRoom);
	CT_VOID	OnUserEnterRoom(acl::aio_socket_stream* pSocket, MSG_PG_EnterRoom* pRoom, CT_BOOL bDistribute = false);
	CT_VOID OnUserEnterRoomEx(acl::aio_socket_stream* pSocket, MSG_PG_EnterRoom* pRoom);
	CT_VOID OnAddUserWaitList(acl::aio_socket_stream* pSocket, MSG_PG_EnterRoom* pRoom);
	CT_VOID OnUserChangeTable(CT_DWORD dwUserID);

private:
	//发送消息
	CT_VOID SendMsg(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen);
	//关闭玩家socket
	CT_VOID CloseUserSocket(acl::aio_socket_stream* pSocket, CT_UINT64 uClientSocket);
	//发送坐下失败
	CT_VOID SendSitFail(acl::aio_socket_stream* pSocket, CT_DWORD dwClientValue, CT_INT32 iErrorCode);
	//发送游戏失败数据
	CT_VOID SendGameErrorCode(acl::aio_socket_stream* pSocket,  CT_WORD wMainID, CT_WORD wSubID, CT_UINT64 uClientValue, CT_INT32 iErrorCode);
	//发送用户信息
	CT_VOID SendUserMsg(CServerUserItem* pUser, MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen);

	//proxy注册这个玩家
	void AddGameUserToProxy(CT_DWORD dwUserID, CT_UINT64 uClientAddr, acl::aio_socket_stream* pProxySock);
	//到proxy删除这个玩家
	void DelGameUserToProxy(CT_DWORD dwUserID, acl::aio_socket_stream* pProxySock);

private:
	CT_VOID SwitchUserItem(CServerUserItem *pUserItem, acl::aio_socket_stream* pProxySock, CT_UINT64 dwClient);

public:
	inline CT_DWORD GetWaitListCount() { return (CT_DWORD)m_listWaitList.size(); }

private:
	tagGameKind			m_GameKind;
	tagGameRoomKind		m_GamePlayKind;
	//CCDBConnector		m_platformDBCon;
	CWXGroupMgr			m_wxGroupMgr;
	ListWaitList		m_listWaitList;			//等待上桌玩家列表
	CT_DWORD			m_dwWaitDistributeTime;	//等待分配时间

	acl::redis_client	m_redisClient;
	acl::redis			m_redis;
	acl::mysql_manager	m_dbManager;

	CT_BOOL				m_bInitServer;
};