#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "Code/Prereqs.h"
#include <map>
//#include <sstream>
#include <random>
#include "Code/GameTable.h"

//游戏流程
class CGameProcess : public ITableFrameSink
{
public:
	CGameProcess(void);
	~CGameProcess(void);
    
public:
	//游戏开始
	virtual void	OnEventGameStart();
	//游戏结束
	virtual void OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag);
	//发送场景
	virtual void OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
    
	//事件接口
public:
	//定时器事件
	virtual void	OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏消息
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);

	//用户接口
public:
	//用户进入
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0);
	//用户断线
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户重入
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户换桌
	virtual void OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce);
	//用户充值 llRechargeScore充值金额 单位: 分
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) {}

public:
	//设置指针
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//清理游戏数据
	virtual void ClearGameData();
	//私人房游戏大局是否结束
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//	获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);
	//设置桌子库存
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {  }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) { }
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {}
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) {}
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) {}
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) {}
	//设置今日库存上限
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) {}
	//设置换牌概率
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) {}
	//设置换牌概率
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio) {}
	//设置游戏黑名单
	virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList);
	//设置基础捕获概率
	virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) {}
	//设置捕鱼点控数据
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) { return true; }
	//设置炸金花的牌库概率
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio){};
	//捕鱼获取桌子上玩家的输赢值
	virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) {}
	//设置血控系统数据(新)
	virtual CT_VOID SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo) {}
	//获取桌子的鱼种统计信息
	virtual CT_VOID GetAllPlayerFishStatisticInfo() {}
    virtual CT_VOID SetJSFishData(enJSFishDataType type, void* pData);
	//捕鱼自己添加的函数
public:
    //获取和设置玩家当前分数
    int64_t get_user_score(unsigned short chair_id);
    void change_user_score(unsigned short chair_id,  int64_t user_score);
    //获取和设置玩家的输赢分数
    int64_t get_user_win_lose_score(unsigned short chair_id);
    void set_user_win_lose_score(unsigned short chair_id, int64_t user_score);

    void broadcast_all(unsigned short subID, void *pBuf, unsigned short data_len);
    void send_player_data(unsigned short chair_id, unsigned short subCmdId, void *pBuffer, unsigned short len);
    bool is_exist_user(unsigned short chair_id);

    uint32_t get_user_id(uint16_t chair_id);
    void set_user_miss_stock(uint16_t  chair_id ,int64_t miss_stock);
    int64_t get_user_miss_stock(uint16_t  chair_id);

    void set_user_stream(uint16_t chair_id, int64_t user_stream);
    int64_t  get_user_stream(uint16_t chair_id);

    void set_user_tax(uint16_t chair_id, int64_t user_tax);
    void set_user_send_count(uint16_t chair_id);

    uint32_t get_total_user_count() {return m_chair_info.size();}
    uint32_t get_room_enter_limit() {return m_pGameDeskPtr->GetGameKindInfo()->dwEnterMinScore;}

    void set_timer(uint16_t timer_id, uint32_t Interval, uint32_t Param = 0);
    //读取配置，服务器自启动只读取一次。
	void read_conf();

	//关服的时候保存系统信息
	void save_sysinfo_shutserver();

	void set_user_last_fire_time(unsigned short chair_id, time_t last_fire_time);
	time_t get_user_last_fire_time(unsigned short chair_id);
private:
	//发送数据
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false);
	void InitAllPlayerInfo(CT_DWORD dwChairID);
	void UnInitAllPlayerInfo(CT_DWORD dwChairID);
private:
	ITableFrame*					m_pGameDeskPtr;							//游戏指针
	CGameLogic						m_GameLogic;							//游戏逻辑
	GameTableLogic 				    *m_pGame_Table_Logic;

	typedef std::map<CT_DWORD, chair_info> map_chair_info;
	map_chair_info m_chair_info;
    static bool m_load_config;
    //是否启动了保存库存的定时器.
	static bool m_start__timer_save_stock;
};

extern Game_Config_Xml g_game_config_xml;
#endif	//___DEF_GAME_PROCESS_H___
