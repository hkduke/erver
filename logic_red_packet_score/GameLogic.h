#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_HB.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include <random>
#include "ITableFrame.h"


//宏定义
//////////////////////////////////////////////////////////////////////////
#define COLOR_POOL_INIT_AMOUNR       (88888)             //彩池初始金额
#define COLOR_POOL_RATIO             (0.1)               //彩池比率
//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))
struct AssignHBInfo
{
    AssignHBInfo()
    {
        init();
    }
    void init()
    {
        memset(bIsThunder,0,sizeof(bIsThunder));
        memset(wChairID,INVALID_CHAIR,sizeof(wChairID));
        memset(dwUserID,0,sizeof(dwUserID));
        memset(llOutHB,0,sizeof(llOutHB));
        memset(llThunderBackAmount,0,sizeof(llThunderBackAmount));
        memset(llSourceScore,0,sizeof(llSourceScore));
        memset(iRevenue,0,sizeof(iRevenue));
        memset(llGrabWinColorPoolAmount,0,sizeof(llGrabWinColorPoolAmount));
        wRemainHBCount = 0;
        llRemainAmount = 0;
        llTotalAmount = 0;
        iStockCtrlType = en_NoCtrl;
        iFaPointCtrlType = en_NoCtrl;
        iRobotThunderCount = 0;
        llFaWinColorPoolAmount = 0;
        iThunderCount = 0;
        iRobotGrabCount = 0;
    }
    bool bIsThunder[MAX_HB_COUNT];                  //是否中雷
    CT_WORD wChairID[MAX_HB_COUNT];                  //座位号
    CT_DWORD dwUserID[MAX_HB_COUNT];                 //UserID
    CT_LONGLONG llOutHB[MAX_HB_COUNT];              //红包金额
    CT_LONGLONG llThunderBackAmount[MAX_HB_COUNT];  //中雷返现金额
    CT_LONGLONG llSourceScore[MAX_HB_COUNT];        //抢此红包玩家的原金币
    CT_INT32 iRevenue[MAX_HB_COUNT];                //税收
    CT_LONGLONG llGrabWinColorPoolAmount[MAX_HB_COUNT];                //抢红包玩家赢的彩池金额
    CT_WORD wRemainHBCount;                         //剩余红包数
    CT_LONGLONG llRemainAmount;                     //剩余金额
    CT_LONGLONG llTotalAmount;                      //总金额
    CT_INT32 iStockCtrlType;                        //库存控制方式
    CT_INT32 iFaPointCtrlType;                       //发红包玩家点控方式
    CT_INT32 iRobotThunderCount;                      //在控制模式下机器人中雷数
    CT_LONGLONG llFaWinColorPoolAmount;                //发红包玩家赢的彩池金额
    CT_INT32 iThunderCount;                             //中雷数
    CT_INT32 iRobotGrabCount;                            //机器人抢的红包包数
};
//用户申请发红包加入扫雷队列
struct tagHBInfo
{
    tagHBInfo()
    {
        memset(&stFaHBInfo, 0, sizeof(stFaHBInfo));
        stAssignHBInfo.init();
        vOpenHBList.clear();
        dwGrabTime = 0;
        dwEndTime = 0;
        llFaSourceScore = 0;                    //发红包玩家的原金币
    }
    CT_DWORD dwGrabTime;                              //发红包时间
    CT_DWORD dwEndTime;                              //发红包结束时间
    GS_ApplyFaHBInfo stFaHBInfo;
    AssignHBInfo stAssignHBInfo;
    std::vector<CMD_S_OpenHB> vOpenHBList;
    CT_LONGLONG llFaSourceScore;                    //发红包玩家的原金币
};
//游戏逻辑类
class CGameLogic
{
	//函数定义
public:
	//构造函数
	CGameLogic();
    bool RandAssignHB(tagHBInfo & stHBInfo);
    CT_INT32 RandAssignHB2(tagHBInfo & stHBInfo, CT_INT32 iGrabHBPlayerCtrlType, CT_BOOL bIsRobot, CT_BOOL bIsFaRobot, CT_BOOL bIsSelfFaSelfGrab, CT_LONGLONG llGrabWinScore, GS_SimpleUserInfo & stGrabUserInfo);
	//析构函数
	virtual ~CGameLogic();
    void SetKillScoreCfg(GS_StockKillScoreCfg & stKillScoreCfg){m_StockKillScoreCfg = stKillScoreCfg;}
    void SetPointCtrlCfg(GS_PointCtrlCfg & stPointCtrlCfg){m_PointCtrlCfg = stPointCtrlCfg;}
    void SetColorPoolCfg(std::vector<GS_ColorPoolCfg> & vColorPoolCfg)
    {
        m_vColorPoolCfg.clear();
        m_vColorPoolCfg.assign(vColorPoolCfg.begin(), vColorPoolCfg.end());
    }
    void SetGameRoomKindInfo(tagGameRoomKind* pGameRoomKindInfo){m_pGameRoomKindInfo = pGameRoomKindInfo;}
    void SetCtrlMasterSwitch(CT_BYTE bCtrlMasterSwitch) { m_bCtrlMasterSwitch = bCtrlMasterSwitch;}
    static void SetColorPool(CT_DOUBLE dColorPool){ m_sdColorPool += dColorPool;}
    static void SetRealColorPool(CT_DOUBLE dRealColorPool){ m_sdRealColorPool += dRealColorPool;}
    static void SetRobotColorPool(CT_DOUBLE dRobotColorPool){ m_sdRobotColorPool += dRobotColorPool;}
    static CT_LONGLONG GetColorPool(){ return (CT_LONGLONG)m_sdColorPool;}
    static CT_LONGLONG GetRealColorPool(){ return (CT_LONGLONG)m_sdRealColorPool;}
    static CT_LONGLONG GetRobotColorPool(){ return (CT_LONGLONG)m_sdRobotColorPool;}
    static CT_LONGLONG GetRealOutColorPool(){ return m_sllRealOutColorPool;}

    CT_DWORD JudgeWinPrize(tagHBInfo & stHBInfo, int i);
    CT_VOID CalculateColorPrize(tagHBInfo & stHBInfo, int i, CT_DWORD iWinPrizeIndex, CT_BOOL bIsRobot, CT_BOOL bIsFaRobot);

private:

    static CT_DOUBLE 				m_sdColorPool;											//彩池
    static CT_DOUBLE 				m_sdRealColorPool;										//真人彩池
    static CT_DOUBLE 				m_sdRobotColorPool;										//机器人彩池
    static CT_DOUBLE 				m_sllRealOutColorPool;									//真人彩池总派彩
    static CT_LONGLONG              m_sllHBAmount[MAX_DISPLAY_HB_COUNT];
    CT_LONGLONG              m_llHBAverageAmount;                                    //平均金额

    GS_StockKillScoreCfg	            m_StockKillScoreCfg;			//机器人库存杀分配置
    GS_PointCtrlCfg	    				m_PointCtrlCfg;					//点控配置
    std::vector<GS_ColorPoolCfg>   		m_vColorPoolCfg;						//彩池配置
    CT_BOOL                             m_bCtrlMasterSwitch;                   //是否控制总开关
    CT_DWORD m_dwRandIndex;
    tagGameRoomKind*				m_pGameRoomKindInfo;
    std::random_device m_rd;
    std::mt19937 m_gen;
    std::uniform_int_distribution<unsigned int> m_iProba;
};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


