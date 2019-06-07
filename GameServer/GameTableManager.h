#pragma once

#include "TableFrame.h"
#include "TableFrameForScore.h"
#include <vector>
#include "GlobalStruct.h"
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Inner.h"

#define	IDI_FISH_BLOODPOOL_STATE_VALUE (108)
#define TIME_FISH_BLOODPOOL_STATE_VALUE 1*1000

#define IDI_SAVE_FISH_CONTROL_DATA (109)
#define TIME_SAVE_FISH_CONTROL_DATA 30*1000//5*60*1000

#define IDI_SAVE_KUCUN_DATA (110)
#define TIME_SAVE_KUCUN_DATA 30*1000

#define IDI_SAVE_STATISTICSFISHINFO (111) //统计鱼种的死亡概率 

#define IDI_SAVE_TODAY_SERVER_INFO (112) //统计服务器当天的税收等信息
#define TIME_SAVE_TODAY_SERVER_INFO 10 * 60 * 1000

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

    tagWaitListInfo& operator =(const tagWaitListInfo& listInfo)
    {
        if (this != &listInfo)
        {
            this->dwUserID = listInfo.dwUserID;
            this->uClientAddr = listInfo.uClientAddr;
            this->pProxySock = listInfo.pProxySock;
        }
        return *this;
    }
};

struct tagStockInfo
{
	CT_LONGLONG						llStorageControl;								//库存控制值,控牌对应值
	CT_LONGLONG						llStorageLowerLimit;							//库存控制下限值
	CT_LONGLONG						llTodayStorageControl;							//今日库存
	CT_LONGLONG						llTodayStorageHighLimit;						//今日库存上限
	CT_LONGLONG						llAndroidStorage;								//机器人库存控制值
	CT_LONGLONG						llAndroidStorageLowerLimit;						//机器人库存
	CT_WORD							wSystemAllKillRatio;							//系统通杀概率(百分比)
	CT_WORD							wChangeCardRatio;								//换牌概率
};

typedef std::vector<tagWaitListInfo>	VecWaitList;
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
	void InitAllTable(CT_DWORD dwTableCount, CGameServerThread* pGsThread, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis, acl::mysql_manager* pdbManager);
	ITableFrame* GetTable(CT_DWORD id);
	CTableFrame* FindOneUnlockTable();
    CTableFrameForScore* FindOneUnlockScoreTable();
    CTableFrame* FindTableByRoomNum(CT_DWORD dwRoomNum);
	CTableFrameForScore * FindTableByGroupRoomNum(CT_DWORD dwRoomNum);


public:
	ITableFrame* FindNormalSuitTable(CT_DWORD wExceptTableID = INVALID_CHAIR);
	void FreeNomalTable(ITableFrame* pTableFrame);

public:
	void OnTimePulse();
	//acl::mysql_manager* GetMysqlManager() { return m_pdbManager; }
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

	CT_DWORD GetGameIndex() { return m_dwGameIndex; }

	VecTableFrame& GetTableFrame() { return m_vecGameTable; }
	ListTableFrame& GetUsedTableFrame() { return m_listUsedTable; }

	//功能函数
public:
	//对百人游戏有效
	CT_VOID SetTableStockInfo(tagStockInfo& stockInfo);
	CT_VOID SetTableStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo); //针对新的血池控制系统
	CT_VOID UpdateTableStockInfo(MSG_C2GS_Update_GameServer_Stock* pStockInfo);
	CT_VOID UpdateTableChangeCardRatio(std::vector<tagBrGameOperateRatio>& vecChangeCardRatio);
	CT_VOID UpdateBrGameBlackList(std::vector<tagBrGameBlackList>& vecGameBlackList);
	CT_VOID UpdateZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio);
	CT_VOID UpdateDianKongInfo(std::vector<MSG_D2CS_Set_FishDiankong>& vecDianKongData);
	CT_VOID UpdateBlackChannel(std::vector<tagBlackChannel>& vecBlackChannel);

public:
	CT_VOID TestWritePlayback();
	CT_VOID TestUpdateUserGem();
	CT_VOID TestPlayCountRecord();
	CT_VOID TestClearingInfo();
	//设置所有捕鱼桌子的基础捕获概率
	CT_VOID SetAllFishTableBaseCapPro(CT_INT32 baseCapPro);
	//给在线玩家设置点控数据
	CT_VOID SetOnlineUserDianKongData(const CT_VOID *pData, CT_DWORD dwDataSize);
	//给在线玩家设置miss数据
	CT_VOID SetOnlineUserMissData(const CT_VOID *pData, CT_DWORD dwDataSize);
	//把捕鱼玩家T出房间
	CT_VOID TickOutAllPlayer();
public:
	void Clear();

	CT_VOID InitAllTableBaseCapPro(int nBaseCapPro);
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

    //CGameServerThread*                      m_pGsThread;                    //线程类
	tagGameKind*							m_pGameKind;					//游戏类型
	tagGameRoomKind*						m_pRoomKind;					//房间类型
	CT_DWORD 								m_dwGameIndex;					//房间索引


	tagStockInfo							m_StockInfo;					//库存相关
};