#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include <set>
#include <map>
#include "GlobalStruct.h"


class CDipThread : public CWorkThread
{
public:
	CDipThread(void);
	~CDipThread(void);

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

	CT_VOID InitServer();

private:
	CT_VOID OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnWebMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

private:
	CT_VOID ReturnCommonToWeb(acl::aio_socket_stream* pSocket, std::string code, std::string strMessage);

private:
	CT_BOOL CheckUserIsOnline(CT_DWORD dwUserID);
	CT_VOID QueryOnlineInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//通知重新加载机器人
	CT_VOID UpdateAndroid(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//通知更新机器人游戏数量
	CT_VOID UpdateAndroidPlayGameCount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//通知重新加载充值渠道信息
	CT_VOID ReloadRechargeChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//通知有新邮件
	CT_VOID HasNewMail(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//插入新邮件
	CT_VOID SendNewSystemMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//充值代理列表
	CT_VOID RechargePromoterList(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//设置帐号状态
	CT_VOID SetAccountStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock); 
	//设置兑换帐号状态
	CT_VOID SetExchangeAccountStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//设置推广代理帐号信息
	CT_VOID SetGeneralizePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//设置客户端渠道的主页
	CT_VOID SetClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新全民代理
	CT_VOID UpdateQuanMinChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新赠送金币
  	CT_VOID UpdateChannelPresentScore(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新游戏库存相关信息
	CT_VOID UpdateStockInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//删除系统消息
	CT_VOID RemoveSystemMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//剔除捕鱼用户
	CT_VOID TickFishUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//百人游戏概率控制
	CT_VOID ReloadBrGameRatioControl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//百人牛牛增加黑名单
    CT_VOID AddBlackListToBrGame(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
    //增加或者修改财神降临活动配置
	CT_VOID UpdateWealthGodComingCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//增加或者修改财神降临活动奖励
	CT_VOID UpdateWealthGodComingReward(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//修改充值渠道的充值金额
	CT_VOID UpdateRechargeAmount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//修改兑换渠道的
	CT_VOID UpdateExchangeAmount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//捕鱼点控数据
	CT_VOID SetUserDianKong(const CT_VOID *pData, CT_DWORD wDataSize, acl::aio_socket_stream *pWebSock);
	//炸金花牌库概率
	CT_VOID UpdateZjhDepotCardRation(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//一元夺宝配置
	CT_VOID UpdateDuoBaoCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//救济金配置
	CT_VOID UpdateBenefitCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//平台代理充值的配置
	CT_VOID UpdatePlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更改短信平台
	CT_VOID UpdateSMSPlatformId(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新红包状态
	CT_VOID UpdateRedPacketStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新红包的房间配置
	CT_VOID UpdateRedPacketRoomCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新红包的期数配置
	CT_VOID UpdateRedPacketIndex(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新客服聊天上传图片链接
	CT_VOID UpdateChatUploadUrl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新公告内容
	CT_VOID UpdateAnnouncementInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新黑渠道信息
	CT_VOID UpdateBlackChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新公会信息
	CT_VOID UpdateGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新公会成员信息
	CT_VOID UpdateGroupUserInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//更新公会等级配置信息
	CT_VOID UpdateGroupLevelCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);
	//重新绑定玩家
	CT_VOID UserBindGroup(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock);

private:
	CT_BOOL				m_bInitServer;

	acl::redis_client	m_redisClient;
	acl::redis			m_redis;
};	