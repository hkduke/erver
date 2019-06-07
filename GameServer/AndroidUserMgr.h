﻿#pragma once
#include "AndroidUserItem.h"
#include <map>
#include <set>
#include <memory>

typedef std::map<CT_DWORD, std::shared_ptr<tagAndroidUserParameter>> mapAndroidParam;
typedef std::list<std::shared_ptr<tagAndroidUserParameter>> listAndroidParam;
typedef std::set<std::shared_ptr<tagAndroidUserParameter>> setAndroidParam;
typedef std::map<CT_DWORD, GS_UserBaseData>	MapAndroidUserBaseData;

class CGameServerThread;
class CDdzAiThread;
class CAndroidUserMgr : public acl::singleton<CAndroidUserMgr>
{
public:
	CAndroidUserMgr();
	~CAndroidUserMgr();

public:
	void Initialization(tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, CGameServerThread* pGameserverThread);

	//时间脉冲
	void OnTimePulse();
	//检测玩家是否需要凑桌
	void OnTimeCheckUserIn();
	//检测斗地主机器人进入房间
	inline void CheckDDZGameAndroidIn();
    //检测金猪纳财机器人进入房间
    inline void CheckJZNCGameAndroidIn();
	//检测普通游戏机器人进入房间
	inline void CheckNormalGameAndroidIn();
	//检测捕鱼机器人进入房间
	inline void CheckFishGameAndroidIn();
	//检测红包扫雷活动的机器人进入房间
	inline void CheckHongBaoSLAndroidIn();
	//插入机器人参数
	void InsertAndroidUserParam(tagAndroidUserParameter* pAndroidParam);
	//清空机器人参数
	void ClearAndroidUserParam();

	//插入用户
	CT_BOOL CreateAndroidUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID, CT_BYTE cbBigJetton);
	// 删除用户
	CT_BOOL	DeleteUserItem(CServerUserItem* pServerUserItem);
	// 查找用户
	CServerUserItem* FindAndroidUserItem(CT_DWORD dwUserID);
	//上线机器人数量
	CT_WORD GetOnlineAndroidCount() { return (CT_WORD)m_AndroidUserMap.size(); }

	//更新在线人数
	inline CT_VOID UpdateOnlineCount();

	//设置机器用户
	CT_VOID SetAndroidUserScore(CT_DWORD dwUserID, CT_LONGLONG llScore);

	//设置机器用户数量
	CT_VOID SetActiveAndroidCount(CT_DWORD dwActiveAndroidCount);
	//获取机器用户数量
	CT_DWORD GetActiveAndroidCount();

	//设置机器人的时间脉冲
	CT_VOID SetAndroidTimePlus(CT_DWORD dwTimePulse) { m_dwAndroidTimePulse = dwTimePulse; }
	//获取机器人的时间脉冲
	inline CT_DWORD GetAndroidTimePlus() { return m_dwAndroidTimePulse; }

	//更新机器人
	CT_VOID UpdateAndroidStatus(CT_DWORD dwUserID, CT_BYTE cbStatus);

public:
	GS_UserBaseData* GetAndroidUserBaseData(CT_DWORD dwUserID);

	//斗地主AI线程
	CT_VOID SetDdzAiThread(CDdzAiThread* pAiThread) { m_pDdzAiThread = pAiThread; }
	CDdzAiThread* GetDdzAiThread() { return m_pDdzAiThread; }

private:
	CServerUserItemMap			m_AndroidUserMap;				
	CServerUserItemArray		m_AndroidUserItemSitting;		//坐下的机器人
	CServerUserItemArray		m_AndroidUserItemFree;			//空闲的机器对象

	//机器人配置表
	setAndroidParam				m_setAndroidParamAll;			//全部机器人列表
	listAndroidParam			m_listAndroidParamStore;		//还没有使用的机器人参数
	mapAndroidParam				m_mapAndroidParamUsed;			//已经使用的机器人参数
	MapAndroidUserBaseData		m_mapAndroidUserData;			//已经使用的机器人基本数据

	tagGameKind*				m_pGameKindInfo;
	tagGameRoomKind*			m_pRoomKindInfo;
	CGameServerThread*			m_pGameServerThread;

	CT_DWORD					m_dwActiveAndroidCount;			//激活的机器人数量
	CT_DWORD					m_dwAndroidTimePulse;			//机器人的时间脉冲


	CDdzAiThread*				m_pDdzAiThread;					//斗地主AI线程

	LpDllNewAndroid				m_pFunNewAndroidSink;
};