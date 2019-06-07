#pragma once

#include "TableFrame.h"
#include "TableFrameForScore.h"
#include <vector>
#include "GlobalStruct.h"
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Inner.h"

struct tagWaitListInfo
{
	CT_DWORD		dwUserID;				//用户ID
	CT_UINT64		uClientAddr;			//用户客户端地址
	acl::aio_socket_stream* pProxySock;		//用户所在的proxy连接

	tagWaitListInfo()
		: dwUserID(0)
		, uClientAddr(0)
		, pProxySock(NULL)
	{
	}
};

struct tagStockInfo
{
	CT_LONGLONG						llStorageControl;								//库存控制值,控牌对应值
	CT_LONGLONG						llStorageLowerLimit;							//库存控制下限值
	CT_LONGLONG						llAndroidStorage;								//机器人库存控制值
	CT_LONGLONG						llAndroidStorageLowerLimit;						//机器人库存
	CT_WORD							wSystemAllKillRatio;							//系统通杀概率(百分比)
};

typedef std::list<tagWaitListInfo>		ListWaitList;
typedef std::vector<ITableFrame*>		VecTableFrame;
typedef VecTableFrame::iterator			IterTableFrame;
typedef std::list<ITableFrame*>			ListTableFrame;

//游戏桌子管理类
class CGameTableManager : public acl::singleton<CGameTableManager>
{
public:
	CGameTableManager();
	~CGameTableManager();

public:
	void InitAllTable(CT_DWORD dwTableCount, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis, acl::mysql_manager* pdbManager);
	ITableFrame* GetTable(CT_DWORD id);
	CTableFrame* FindOneUnlockTable();
	CTableFrame* FindTableByRoomNum(CT_DWORD dwRoomNum);

public:
	ITableFrame* FindNormalSuitTable(CT_DWORD wExceptTableID = INVALID_CHAIR);
	void FreeNomalTable(ITableFrame* pTableFrame);

public:
	void OnTimePulse();
	acl::mysql_manager* GetMysqlManager() { return m_pdbManager; }
	acl::db_pool* GetAccountdbPool() 
	{
		acl::db_pool* pool = (acl::db_pool*)m_pdbManager->get(m_accountDBKey.c_str(), false, true);
		return pool;
	}
	acl::db_pool* GetRecorddbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_pdbManager->get(m_recordDBKey.c_str(), false, true);
		return pool;
	}
	acl::db_pool* GetPlatformdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_pdbManager->get(m_platformDBKey.c_str(), false, true);
		return pool;
	}

public:
	CT_VOID InsertGameRoundRewardCfg(CT_DWORD dwGameRoundCount, CT_DWORD dwReward);
	CT_BOOL CheckGameRoundReward(CT_DWORD dwGameRoundCount, CT_DWORD& dwReward);

	CT_VOID GoCenterUpdatePRoom(CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum, CT_DWORD dwGroupID, CT_BYTE cbGroupType, CT_BOOL bRemoveGroupRoom, CT_BYTE cbRoomState);
	CT_VOID SendBackGroupPRoomGem(CT_DWORD dwGroupID, CT_DWORD dwRecordID, CT_DWORD dwMasterID, CT_WORD wGem);
	CT_VOID SendBackWXGroupPRoomGem(CT_DWORD dwGroupID, CT_DWORD dwRecordID);

	CT_WORD GetPRoomNeedGem(CT_WORD wPlayCount)
	{
		auto it = m_mapPRoomNeedGem.find(wPlayCount);
		if (it != m_mapPRoomNeedGem.end())
		{
			return it->second;
		}
		return 0;
	}

	VecTableFrame& GetTableFrame() { return m_vecGameTable; }
	ListTableFrame& GetUsedTableFrame() { return m_listUsedTable; }

	//功能函数
public:
	//对百人游戏有效
	CT_VOID SetTableStockInfo(tagStockInfo& stockInfo);

public:
	CT_VOID TestWritePlayback();
	CT_VOID TestUpdateUserGem();
	CT_VOID TestPlayCountRecord();
	CT_VOID TestClearingInfo();
	
public:
	void Clear();

private:
	VecTableFrame	m_vecGameTable;
	ListTableFrame	m_listFreeTable;		//对于金币房的空闲桌子列表
	ListTableFrame	m_listUsedTable;		//对于金币房的已经使用的桌子
	
	acl::mysql_manager* m_pdbManager;
	acl::string		m_accountDBKey;
	acl::string		m_recordDBKey;
	acl::string		m_platformDBKey;

	std::map<CT_DWORD, CT_DWORD>			m_mapGameRoundReward;			//大局奖励
	std::map<CT_WORD, CT_WORD>				m_mapPRoomNeedGem;				//开私人房间需要的钻石

	tagGameKind*							m_pGameKind;					//游戏类型
	tagGameRoomKind*						m_pRoomKind;					//房间类型

	tagStockInfo							m_StockInfo;					//库存相关
};