#pragma once

#include "CMD_Game.h"
#include "acl_cpp/lib_acl.hpp"
#include <deque>
#include <map>
#include "IServerUserItem.h"
#include "IAndroidUserItemSink.h"
#include "GlobalStruct.h"
#include "IGameOtherData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//玩家私人房间信息
typedef struct STR_GAME_UserPRoomData
{
	CT_DWORD		dwPRoomNum;							//私人房间信息
	CT_DWORD		dwGameRoundCount;					//私人房间大局数
}GS_UserPRoomData;

//游戏用户
class CServerUserItem : public IServerUserItem
{
public:
	CServerUserItem();
	virtual ~CServerUserItem();

public:
	//是否机器人
	virtual bool IsAndroid() { return false; }

	//获取用户回调对象
	virtual IAndroidUserItemSink* GetAndroidUserItemSink() { return NULL; }

	//重置数据
	virtual void ResetUserItem();
	//定时器脉冲
	virtual void OnTimePulse() {};
	//设置定时器
	virtual bool SetTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0) { return false; }
	//销毁定时器
	virtual bool KillTimer(CT_WORD wTimerID) { return false; }
	//判断是否有定时器
	virtual bool IsHasTimer(CT_WORD wTimerID) { return false; }

	//获得玩家UserID
	inline CT_DWORD GetUserID() { return m_UserBaseData.dwUserID;  }

	//获得玩家平台id
	inline CT_DWORD GetPlatformID() { return m_UserBaseData.dwPlatformID; }

	//获得玩家金币
	inline CT_LONGLONG GetUserScore() { return m_UserBaseData.llScore; }
	//设置玩家金币
	inline CT_VOID SetUserScore(CT_LONGLONG llScore) { m_UserBaseData.llScore = llScore; }
	//获取玩家的银行金币
	inline CT_LONGLONG GetUserBankScore() { return  m_UserBaseData.llBankScore; }

	//获取玩家充值次数
	inline CT_DWORD GetUserTempRechargeCount() { return  m_UserBaseData.dwRechargeTempCount; }
	//设置玩家充值次数
	inline CT_VOID  SetUserTempRechargeCount(CT_DWORD dwCount) { m_UserBaseData.dwRechargeTempCount = dwCount; }

	//获得玩家VIP 
	inline CT_BYTE GetUserVip() { return m_UserBaseData.cbVipLevel; }
	//设置玩家VIP
	inline CT_VOID SetUserVip(CT_BYTE cbVip) { m_UserBaseData.cbVipLevel = cbVip; }

	//获得玩家VIP 
	inline CT_BYTE GetUserVip2() { return m_UserBaseData.cbVipLevel2; }
	//设置玩家VIP
	inline CT_VOID SetUserVip2(CT_BYTE cbVip2) { m_UserBaseData.cbVipLevel2 = cbVip2; }

	//获取玩家IP
	inline CT_CHAR* GetUserIp() { return m_UserBaseData.szIP; }

	//获取玩家地理位置
	inline CT_CHAR* GetUserLocation() { return m_UserBaseData.szLocation; }

	//设置桌子id
	inline void	 SetTableID(CT_WORD dwTableID) { m_wTableID = dwTableID; }
	//得到桌子id
	inline CT_WORD GetTableID() { return m_wTableID; }

	//设置椅子id
	inline void     SetChairID(CT_WORD chairID) { m_wChairID = chairID; }
	//得到椅子id
	inline virtual CT_WORD GetChairID() { return m_wChairID; }

	//设置用户状态
	inline void		SetUserStatus(USER_STATUS us) { m_status = us; }
	//得到用户状态
	inline USER_STATUS GetUserStatus() { return m_status; }

	//是否离开了
	inline CT_BOOL	IsGetOut() { return (sGetOut == m_status) ? CT_TRUE : CT_FALSE; }
	//是否坐下了
	inline CT_BOOL	IsSit() { return (sSit == m_status) ? CT_TRUE : CT_FALSE; }
	//是否准备了.
	inline CT_BOOL	IsReady() { return (sReady == m_status) ? CT_TRUE : CT_FALSE; }
	//是否游戏中.
	inline CT_BOOL	IsPlaying() { return (sPlaying == m_status) ? CT_TRUE : CT_FALSE; }
	//是否断线了.
	inline CT_BOOL	IsBreakLine() { return (sOffLine == m_status) ? CT_TRUE : CT_FALSE; }
	//是否旁观了.
	inline CT_BOOL	IsLookOn() { return (sLookOn == m_status) ? CT_TRUE : CT_FALSE; }
	//是否离开了还在游戏中
	inline CT_BOOL	IsGetOutAtPlaying() { return (sGetOutAtPlaying == m_status) ? CT_TRUE : CT_FALSE; }

	//好牌数据
	//设置炸金花局数
	//inline CT_VOID  SetPlayCount(CT_DWORD dwZjhCount) {  m_UserScoreData.dwPlayCount = dwZjhCount; }
	//获取炸金花局数
	//inline CT_DWORD GetPlayCount() { return m_UserScoreData.dwPlayCount; }
	//炸金花的好牌参数
	/*inline CT_VOID  SetZjhParam(CT_BYTE cbParam) { m_UserBaseData.cbZjhParam = cbParam; }
	//获取炸金花的好牌参数
	inline CT_BYTE  GetZjhParam() { return m_UserBaseData.cbZjhParam; }
	//设置抢庄牛牛局数
	inline CT_VOID SetQznnCount(CT_DWORD dwQznnCount) {  m_UserBaseData.dwQznnCount = dwQznnCount; }
	//获取抢庄牛牛局数
	inline CT_DWORD GetQznnCount() { return m_UserBaseData.dwQznnCount; }
    //炸金花的好牌参数
    inline CT_VOID  SetQznnParam(CT_BYTE cbParam) { m_UserBaseData.cbQznnParam = cbParam; }
    //获取炸金花的好牌参数
    inline CT_BYTE  GetQznnParam() { return m_UserBaseData.cbQznnParam; }*/

public:
	//设置托管状态
	inline void     SetTrustShip(CT_BOOL ts) { m_bTrusteeship = ts; }
	//得到托管状态
	inline CT_BOOL  GetTrustShip() { return m_bTrusteeship; }

	//设置用户网络接口
	inline void	SetUserCon(acl::aio_socket_stream*	pSocket) { m_pSocket = pSocket; }
	//得到用户网络接口
	inline acl::aio_socket_stream* GetUserCon() { return m_pSocket; }

	//设置client网络地址
	inline void SetClientNetAddr(CT_UINT64 uNetAddr) { m_uClientNetAddr = uNetAddr; }
	//获得client网络地址
	inline CT_UINT64 GetClientNetAddr() { return m_uClientNetAddr; }

	//设置db缓冲的接口
	inline void   SetDBCon(acl::aio_socket_stream* pSocket) { m_pDBSocket = pSocket; }
	//得到db缓冲的接口
	inline acl::aio_socket_stream* GetDBCon() { return m_pDBSocket; }

	//设置用户基本数据
	void SetUserBaseData(const GS_UserBaseData&  udb);
	//得到用户基本数据
	GS_UserBaseData& GetUserBaseData();

	//获取玩家的钻石
	CT_DWORD GetUserGem() { return m_UserBaseData.dwGem; }
	void SetUserGem(CT_DWORD dwGem) { m_UserBaseData.dwGem = dwGem; }

	//获取玩家的昵称
	const CT_CHAR* GetUserNickName() { return m_UserBaseData.szNickName; }

	//获取头像索引
	CT_BYTE	GetHeadIndex() { return m_UserBaseData.cbHeadIndex; }
	//获取性别
    CT_BYTE	GetGender() { return m_UserBaseData.cbGender; }

	//设置用户游戏数据
	void SetUserScoreData(const GS_UserScoreData& ugs);
	//得到用户游戏数据
	GS_UserScoreData& GetUserScoreData();

	//设置用户的私人房间信息
	void SetUserPRoomData(const GS_UserPRoomData& proomData);
	//设置私人房间id
	inline void     SetPRoomNum(CT_DWORD dwPRoomNum) { m_UserPRoomData.dwPRoomNum = dwPRoomNum; }
	//得到私人房间id
	inline CT_DWORD GetPRoomNum() { return  m_UserPRoomData.dwPRoomNum; }
	//设置私人房大局数
	inline void SetGameRoundCount(CT_DWORD dwGameRoundCount) { m_UserPRoomData.dwGameRoundCount = dwGameRoundCount; }
	//获取私人房大局数
	inline CT_DWORD GetGameRoundCount() { return m_UserPRoomData.dwGameRoundCount; }

	//获得自己基本数据
	void GetUserBaseMsgData(MSG_SC_UserBaseInfo& basedata);
	//获取自己的基本数据
	void GetUserBaseMsgDataEx(MSG_SC_UserBaseInfo_ForScore& basedata);
	//获取自己的基本数据
	void GetUserBaseMsgDataEx2(MSG_SC_UserBaseInfo_ForScoreEx& basxedata);

	//发送用户信息
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID);
	//发送用户信息
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len);

	//发送用户信息
	void SendDBMessage(CT_DWORD mainID, CT_DWORD subID);
	//发送用户信息
	void SendDBMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len);

	//设置打了多少局
	virtual void AddPlayCount() {  }
	//获取打了多少局
	virtual CT_WORD GetPlayCount() { return 0; }

	//设置大额度机器人
	virtual CT_VOID SetBigJettionAndroid(CT_BYTE cbBigJetton) {}
	//获得是否大额度机器人
	virtual CT_BYTE GetBigJettonAndroid() { return  0; }

private:
	CT_WORD				m_wTableID;						//桌子ID
	CT_WORD				m_wChairID;						//椅子ID
	//CT_DWORD			m_dwPRoomNum;					//私人房间ID

	USER_STATUS			m_status;						//用户状态
	CT_BOOL				m_bTrusteeship;					//是否托管

	acl::aio_socket_stream*	m_pSocket;					//玩家连接
	acl::aio_socket_stream* m_pDBSocket;				//DB连接
	CT_UINT64				m_uClientNetAddr;			//玩家客户端连接地址

	GS_UserBaseData		m_UserBaseData;
	GS_UserScoreData	m_UserScoreData;
	GS_UserPRoomData	m_UserPRoomData;
};

typedef std::list<CServerUserItem*> CServerUserItemArray;
typedef std::map<CT_DWORD, CServerUserItem*> CServerUserItemMap;
class CGameServerThread;
//用户管理类
class CServerUserManager : public acl::singleton<CServerUserManager>
{
public:
	CServerUserManager();
	~CServerUserManager();

protected:
	CServerUserItemMap			m_UserMap;
	CServerUserItemArray		m_UserItemArray;
	CServerUserItemArray		m_UserItemStore;

	tagGameRoomKind*			m_pGameRoomKindInfo;
	acl::redis*                 m_pRedis;

public:
	//初始化
	CT_VOID Init(tagGameRoomKind* pGameRoomKind, acl::redis* pRedis);
	// 插入用户
	CT_BOOL InsertUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID);
	// 删除用户
	CT_BOOL	DeleteUserItem(CServerUserItem* pServerUserItem);
	// 查找用户
	CServerUserItem* FindUserItem(CT_DWORD dwUserID);
	//去中心服务器更新人数
	CT_VOID UpdateUserCount(CT_DWORD dwUserID, CT_BYTE cbUpdateMode);
	//代理服掉线，把玩家设置为离线状态
	CT_VOID SetUserOffLine(acl::aio_socket_stream* pSocket, CGameServerThread* pServerThread);
	//更新玩家的好牌信息
	inline CT_VOID UpdateGoodCardInfo(CServerUserItem* pUserItem);
	//玩家的金币汇总数据
	inline CT_VOID UpdateUserScoreData(CServerUserItem* pUserItem);
	size_t GetTotalUserCnt() { return m_UserMap.size(); }
};


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif