#ifndef		___MESSAGE_DDZ_H_DEF___
#define		___MESSAGE_DDZ_H_DEF___

#include "CGlobalData.h"
#include <string.h>
#include <vector>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//服务定义
//////////////////////////////////////////////////////////////////////////////////
//游戏属性
#define KIND_ID						9										//游戏 I D
#define GAME_NAME					TEXT("红包扫雷")							//游戏名字


//////////////////////////////////////////////////////////////////////////////////

//结算
#define SCORE_TYPE_LOSE				1										//积分类型(输)
#define SCORE_TYPE_WIN				2										//积分类型(赢)


//////////////////////////////////////////////////////////////////////////////////
//场景消息
#define SC_GAMESCENE_FREE					2001							//空闲场景消息
#define SC_GAMESCENE_PLAY					2002							//游戏中场景消息
#define SC_GAMESCENE_END					2003							//游戏结束
#define SC_GAMESCENE_GAME					2004							//游戏状态（乱斗场）
//状态定义
#define GAME_SCENE_FREE						101								//等待开始
#define GAME_SCENE_PLAY						102								//游戏进行
#define GAME_SCENE_END						103								//游戏结束
#define GAME_SCENE_GAME						104								//游戏（乱斗场）

//胜负信息
#define GAME_WIN							0								//胜
#define GAME_LOSE							1								//负


#define GAME_PLAYER								100								//游戏人数
#define MAX_HB_COUNT							10								//红包数量(每个红包最多分成几个子包)
#define MAX_LEVEL							    6								//最大场次
#define TO_SCORE                                100
#define RAND(a,b,c) 							(((rand() + (c))% ((b)-(a)))+ (a))      //随机一个数在a,b范围内的数，c 是防止循环内调用每次随机的值都是相同的
#define RAND_NUM(a)                             (rand()+(a))                    //随机数
#define MAX_FA_HB_COUNT                         20                              //每人发红包的最多个数
#define MAX_DISPLAY_HB_COUNT                    50                              //显示红包最大数量
#define RAND_THUNDER_NO                         10                              //随机雷号
#define MAX_APPLY_LIST_LENTH                    1000                            //申请列表长度
#define MAX_VIP_LEVEL                           7                               //VIP等级最大值
#define MAX_CONFIG                              4                               //配置项最大
////////////////////////////////////////////////////////////////////////////////////////////
//玩家控制方式
enum en_PlayCtrlType
{
    en_NoCtrl   = 0,                            //不点控
    en_PlayerEatScore = -1,                    //玩家吃分
    en_PlayerOutScore = 1,                   //玩家放分
};
//血池状态
enum en_StockStatus
{
	en_EatScore = 1,                    //吃分
	en_OutScore = -1,                   //放分
};
/////////////////////////////////////////////////////////////////////////////////////////////
//玩家基础数据
typedef struct GAME_SimpleUserInfo
{
	GAME_SimpleUserInfo()
	{
		dwUserID = 0;							//用户id
		cbGender = 0;							//性别
		cbVipLevel = 0;							//VIP等级
		cbHeadIndex = 0;						//头像索引
		bRobot = false;
        llScore = 0;
		llSourceScore = 0;
        llWinLoseScore = 0;
        dwFirstTime = 0;
        dwPointCtlEndTime = 0;
        llGrabContinueWinScore = 0;
        llGrabContinueLoseScore = 0;
		dwContinueGrabLastOneHB = 0;
	}
	CT_DWORD		dwUserID;							//用户id
	CT_BYTE         cbGender;							//性别
	CT_BYTE			cbVipLevel;							//VIP等级
	CT_BYTE			cbHeadIndex;						//头像索引
	CT_BOOL 		bRobot;								//机器人
	CT_LONGLONG     llScore;
	CT_LONGLONG		llSourceScore;						//原金币
    CT_LONGLONG		llWinLoseScore;                     //累计输赢
    CT_DWORD        dwFirstTime;                        //首次加入时间
    CT_DWORD        dwPointCtlEndTime;                  //点控结束时间
    CT_LONGLONG        llGrabContinueWinScore;             //抢红包连续赢的钱
    CT_LONGLONG        llGrabContinueLoseScore;            //发红包连续输的钱
	CT_DWORD		dwContinueGrabLastOneHB;			//连续抢最后一个红包的次数
}GS_SimpleUserInfo;
//被抢红包玩家信息
typedef struct GAME_PlayerBaseData
{
    GAME_PlayerBaseData()
    {
        dwUserID= 0;
        cbGender = 0;
        cbVipLevel = 0;
        cbHeadIndex = 0;
        cbIsThunder = 0;
        dScore = 0;
        dWinningAmount = 0;
        dWinLoseScore = 0;
    }

    CT_DWORD		dwUserID;							//用户id
    CT_BYTE         cbGender;							//性别
    CT_BYTE			cbVipLevel;							//VIP等级
    CT_BYTE			cbHeadIndex;						//头像索引
    CT_BYTE			cbIsThunder;						    //是否中雷
    CT_DOUBLE		dScore;							    //金币
    CT_DOUBLE		dWinningAmount;						//抢的红包金额
    CT_DOUBLE		dWinLoseScore;					            //本轮扣税后的金额
}GS_PlayerBaseData;
//用户申请发红包加入扫雷队列
typedef struct GAME_ApplyFaHBInfo
{
    GAME_ApplyFaHBInfo()
    {
        dwHBID = 0;
        dwUserID = 0;
        wChairID = INVALID_CHAIR;
        cbThunderNO = 0;
        iHBAmount = 0;
        iHBAllotCount = 10;
        dMultiple = 1;
    }
    CT_DWORD                        dwHBID;                              //每个红包ID
    CT_DWORD		                dwUserID;							//用户id
    CT_WORD							wChairID;							//玩家id
    CT_BYTE							cbThunderNO;						//中雷号
    CT_INT32						iHBAmount;							//红包金额
	CT_INT32						iHBAllotCount;						//红包包数
	CT_DOUBLE  						dMultiple;							//倍数
}GS_ApplyFaHBInfo;
//在线玩家战绩统计
typedef struct GAME_OnlinePlayerExploits
{
    CT_DWORD		                dwUserID;							//用户id
    CT_WORD							wPlayCount;						    //抢红包次数
    CT_WORD						    wWinCount;							//赢的次数
    CT_DOUBLE                       dWinLoseScore;                      //输赢金币
	CT_BYTE         				cbGender;							//性别
	CT_BYTE							cbVipLevel;							//VIP等级
	CT_BYTE							cbHeadIndex;						//头像索引
}GS_OnlinePlayerExploits;
//黑名单
struct tagHBGameBlackList
{
	CT_DWORD 		dwUserID;							//黑名单用户ID
	CT_WORD 		wUserLostRatio;						//用户输的概率
	CT_LONGLONG    	llControlScore;                     //控制的下注额度
	CT_LONGLONG    	llCurrControlScore;                 //当前控制的下注额度
};
///////////////////////////////////////////////////////////////////////////////////////////
//场次配置
typedef struct GAME_HBRoomBaseCfg
{
    CT_INT32  iHBAllotCount;					//红包包数
    CT_DOUBLE  fMultiple;				//倍数
    CT_DWORD  dwHBLowerLimit;			    //红包金额下限(单位是元)
    CT_DWORD  dwHBUpperLimit;			    //红包金额上限(单位是元)
}GS_HBRoomBaseCfg;
//机器人抢红包配置
typedef struct GAME_RobotGrabHBTable
{
    CT_INT32  iRealLowerLimit;					//真人抢红包下线
    CT_INT32  iRealUpperLimit;					//真人抢红包上线
    CT_INT32  iRobotLowerLimit;					//机器人抢红包下线
    CT_INT32  iRobotUpperLimit;					//机器人抢红包上线
    CT_INT32  iRobotTimeLowerLimit;				//机器人抢红包时间下线
    CT_INT32  iRobotTimeUpperLimit;				//机器人抢红包时间上线
}GS_RobotGrabHBTable;
typedef struct GAME_RobotGrabHBCfg
{
    CT_INT32  iHBAllotCount;                         //红包数量（场次）
    std::vector<GS_RobotGrabHBTable> vRobotGrabHBTable;
}GS_RobotGrabHBCfg;
//机器人发红包配置
typedef struct GAME_RobotFaHBAmountTable
{
	CT_INT32    iHBMinAmountMultipleLowerLimit;				//该场次红包最小金额的多少倍下线(1-30)
	CT_INT32    iHBMinAmountMultipleUpperLimit;				//该场次红包最小金额的多少倍上线(1-30)
    CT_INT32    iProba;                             //概率
}GS_RobotFaHBAmountTable;
//机器人发红包金额配置
typedef struct GAME_RobotFaHBAmountCfg
{
    CT_INT32    iHBAllotCount;                           //红包数量（场次）
    std::vector<GS_RobotFaHBAmountTable> vFaHBAmount;
}GS_RobotFaHBAmountCfg;
//机器人申请发红包配置
typedef struct GAME_RobotApplyFaHBCfg
{
    CT_INT32    iApplyListLenLowerLimit;            //申请列表长度
    CT_INT32    iApplyListLenUpperLimit;            //申请列表长度上限
    CT_INT32    iProba;                             //申请发红包概率
    CT_INT32    iFaHBRobotCount;                    //申请发红包的机器人数量
}GS_RobotApplyFaHBCfg;

//机器人退出配置
typedef struct GAME_RobotQuitTable
{
    CT_INT32    iOnlineTime;                        //在线时间
    CT_INT32    iQuitProba;                         //退出概率
}GS_RobotQuitTable;
//机器人退出配置
typedef struct GAME_RobotQuitCfg
{
    std::vector<GS_RobotQuitTable> vQuitTable;
	CT_LONGLONG    llRemainScore;                       //游戏剩余金币(单位是分)
}GS_RobotQuitCfg;
//机器人触发杀分配置
typedef struct GAME_TriggerKillScoreTable
{
    CT_LONGLONG    llTriggerStockLowerLimit;            //库存触发杀分下限(单位是分)
    CT_LONGLONG    llTriggerStockUpperLimit;            //库存触发杀分上限(单位是分)
	CT_INT32    iKillScoreProba;                    	//杀分概率
	CT_INT32    iCtrlType;                            	//控制机器人赢1，控制机器人输-1，0不控制
}GS_TriggerKillScoreTable;
//机器人触发杀分配置
typedef struct GAME_TriggerKillScoreCfg
{
	std::vector<GS_TriggerKillScoreTable> vKillScoreTable;
    CT_INT32    iRealFaHBRobotThunderProba;                    //系统杀分阶段，真人发红包其他机器人中雷概率
	CT_INT32	iSysOutScorRealFaHBRobotThunderProba;			//系统吐分阶段，真人发红包其他机器人中雷概率
	CT_INT32    i5HBLevelRobotFaRealThunderProba;                        //杀分阶段，机器人发红包5包场中雷概率
    CT_INT32    i7HBLevelRobotFaRealThunderProba;                        //杀分阶段，机器人发红包7包场中雷概率
    CT_INT32    i10HBLevelRobotFaRealThunderProba;                       //杀分阶段，机器人发红包10包场中雷概率
}GS_StockKillScoreCfg;
//批量发红包个数与VIP配置
typedef struct GAME_BatchHBCountVIPCfg
{
    CT_INT32    iVIPLevel;                      //vipLevel
    CT_INT32    iMaxFaHB;                       //每次发红包的最大个数
}GS_BatchHBCountVIPCfg;
//按钮显示配置
typedef struct GAME_HBButtonCfg
{
    CT_INT32    iCommonlyUsedAmount[MAX_CONFIG];                       //常用金额
    CT_INT32    iCommonlyUsedCount[MAX_CONFIG];                         //常用次数
}GS_HBButtonCfg;
//点控配置
typedef struct GAME_PointCtrlTable
{
	CT_LONGLONG     llChgScoreLowerLimit;                       //赢钱下线(单位是分)
	CT_LONGLONG     llChgScoreUpperLimit;                       //赢钱上线(单位是分)
	CT_INT32        iProba;                                 //控制概率
	CT_INT32        iCtlChgPstLowerLimit;                   //控制改变分数百分比下线
	CT_INT32        iCtlChgPstUpperLimit;                   //控制改变分数百分比上线
	CT_INT32        iCtrlType;                              //控制玩家输-1，控制玩家赢+1，
}GS_PointCtrlTable;
//点控配置
typedef struct GAME_PointCtrlCfg
{
	CT_INT32			iExeProba;							//发红包玩家点控执行概率
    CT_INT32			iTimeInterval;						//时间间隔
    CT_INT32			iGrabBaseProbaMultiple;				//抢红包玩家点控执行概率（2-10）
	std::vector<GS_PointCtrlTable> 	vPointCtrlTable;
}GS_PointCtrlCfg;
//点控配置
typedef struct GAME_RobotFaHBAllotCfg
{
    CT_INT32        iHBAllotCount;                       //红包种类
    CT_INT32        iProba;                                 //概率
}GS_RobotFaHBAllotCfg;

//彩池配置
typedef struct GAME_ColorPoolCfg
{
    std::vector<CT_INT32>       vWinPrizeNumber;                    //中奖数字
    CT_INT32        iGrabWinPrizeRatio;                               //抢红包获奖金额百分比
    CT_INT32        iFaWinPrizeRatio;                                 //发红包获奖金额百分比
    CT_LONGLONG     llMinWinPrizeAmount;                             //最低获奖金额
    CT_INT32        iPointCtlLoseWinProba;                          //点控输玩家中彩的化是否派菜概率
    CT_INT32        iRobotWinPrizeProba;                          //机器人中彩的化是否派菜概率
	CT_INT32        iWinPrizeProba;                          		//中彩的化是否派菜概率
}GS_ColorPoolCfg;
//黑名单配置
typedef struct GAME_BlackListCfg
{
	GAME_BlackListCfg():
			iContinueGrabLastOneHB(15)
			,iCtrlLoseProba(50)
			,fCtrlLoseScoreRatio(0.65)
	{

	}
	CT_INT32        iContinueGrabLastOneHB;                         //连续抢多少次最后一个红包加入黑名单
	CT_INT32        iCtrlLoseProba;                          		//默认加入黑名单的用户控制输的概率
	CT_FLOAT		fCtrlLoseScoreRatio;							//控制黑名单玩家输其总赢的比率
}GS_BlackListCfg;
////////////////////////////////////////////////////////////////////////////////////////////
//空闲状态
struct CMD_S_StatusFree
{
	//游戏属性
    GS_HBRoomBaseCfg		    stHBRoomBaseCfg;						    //基础配置
    CT_WORD						dwCurrPlayCount;                            //当前在线玩家数量
    GS_BatchHBCountVIPCfg       stVIPFaHBcfg;
    GS_HBButtonCfg              stHBButtonCfg;                              //客户端按钮显示配置
};
//游戏状态
struct CMD_S_StatusPlay
{
    CT_WORD						    wCurrFaHBChairID;					    //当前发红包玩家
    GS_PlayerBaseData               stFaHBPlayer;                           //发红包玩家信息
    //时间信息
	CT_WORD							wLeftTime;								//剩余时间
    CT_BYTE							cbThunderNO;						    //中雷号
    CT_INT32						iFaHBTotalAmount;					    //红包总金额
    GS_HBRoomBaseCfg		        stHBRoomBaseCfg;						    //基础配置
    CT_BYTE					        cbSurplusHBCount;						//剩余未被抢红包数量
    GS_PlayerBaseData               stAlreadyGrabHBPlayer[MAX_HB_COUNT];    //已经抢过红包的玩家
    GS_BatchHBCountVIPCfg           stVIPFaHBcfg;
    CT_DWORD                         dwHBID;                                  //红包ID
    GS_HBButtonCfg              stHBButtonCfg;                              //客户端按钮显示配置
};
//游戏结束
struct CMD_S_StatusEND
{
    CT_WORD						wCurrFaHBChairID;					        //当前发红包玩家
    CT_INT32					iFaHBTotalAmount;					        //红包总金额
    GS_HBRoomBaseCfg		    stHBRoomBaseCfg;						    //基础配置
    CT_BYTE						cbTimeLeave;								//剩余时间
    CT_BYTE						cbNoGrabHBCount;							//没抢的红包数量
    CT_DOUBLE					dBackTotalAmount;							//退回的总金额
    CT_DOUBLE					dGrabedTotalAmount;							//被抢的总金额
    CT_BYTE					    cbGrabedHBCount;							//被抢红包数量
    CT_DOUBLE		            dThunderBackTotalAmount;					//中雷返现总金额
    GS_PlayerBaseData           stGrabHBPlayer[MAX_HB_COUNT];               //抢过红包的玩家
    GS_BatchHBCountVIPCfg       stVIPFaHBcfg;
    GS_HBButtonCfg              stHBButtonCfg;                              //客户端按钮显示配置
};
//游戏状态（乱斗场只有一个状态）（乱斗场专用）
struct CMD_S_GamePlay
{
    GS_BatchHBCountVIPCfg           stVIPFaHBcfg;
	GS_HBRoomBaseCfg		        stHBRoomBaseCfg[MAX_CONFIG];
    CT_DOUBLE                       dTotalColorPotAmount;                     //彩池金额
    GS_HBButtonCfg                  stHBButtonCfg;                             //客户端按钮显示配置
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_S_GRAB_HB			    100										//广播抢红包
#define SUB_S_REVOKE_FA_HB			101										//用户撤销发红包服务器回复
#define SUB_S_OPEN_HB				103									    //用户打开红包服务器回复（成功广播，失败单播
#define SUB_S_APPLY_BATCH_FA_HB 	104								        //用户申请批量发红包
#define SUB_S_APPLY_LIST			105									    //申请发红包列表
#define SUB_S_GAME_CONCLUDE			107									    //游戏接受结算
#define SUB_S_RANKING_LIST		        108								    //获取在线玩家排行榜（按抢红包次数排序）
#define SUB_S_NOTICE_ROBOT_GRAB_HB			110							    //通知机器人抢红包
#define SUB_S_NOTICE_ROBOT_FA_HB		    111								//通知机器人申请发红包
#define SUB_S_GET_HB_GRABED_INFO		    115								//获取红包被抢信息服务器回复
#define SUB_S_DELETE_DISPLAY_HB		        116								//删除显示列表中的红包
#define SUB_S_RANKING_LIST_BY_WINLOSE		119								//获取在线玩家排行榜（按抢金额排序
#define SUB_S_SEND_DISPLAY_HB               120                             //发送正在显示的红包
#define SUB_S_RANKING_LIST_BY_COUNT		    121								//获取在线玩家排行榜（按抢次数排序
#define SUB_S_NEXT_FA_HB_PLAYER				122								//广播下一个发红包玩家信息（CMD_S_PlayerApply）
//玩家申请list，一次发送多个此结构体
struct CMD_S_PlayerApply
{
	CT_DWORD		dwUserID;							//用户id
	CT_WORD			wChairID;							//发红包玩家id
	CT_BYTE         cbGender;							//性别
	CT_BYTE			cbVipLevel;							//VIP等级
	CT_BYTE			cbHeadIndex;						//头像索引
    CT_DWORD        dwHBID;                             //红包ID
	CT_BYTE			cbThunderNO;						//中雷号
    CT_INT32		iFaHBTotalAmount;					//红包总金额
    CT_INT32        iHBAllotCount;                      //发的红包可以分配成多少个子包（包数
    CT_DOUBLE       fMultiple;				            //倍数
};
//广播抢红包
struct CMD_S_StartGameGrabHB
{
	CMD_S_PlayerApply				stPlayerApply;
	CT_WORD							wTimeLeft;								//剩余时间
};

//广播用户打开红包服务器
struct CMD_S_OpenHB
{
    CT_BYTE 						cbIsFail;								//是否失败，0成功，1已经领过红包了不能重复，2红包抢完了，3你身上钱不足不能抢红包
    CT_WORD							wChairID;							    //申请开红包玩家id
	CT_DWORD						dwUserID;							    //用户id
	CT_DOUBLE				 		dWinningAmount;						    //中红包金额
    CT_BYTE							cbIsThunder;						    //是否中雷
    CT_DOUBLE                       dWinColorPotAmount;                     //获得彩池金额
    CT_DOUBLE						dWinLoseScore;					        //本轮扣税后的金额
    CT_DWORD                         dwHBID;                                 //红包ID
    CT_DOUBLE				 		dTotalWinLose;						    //总输赢
    CT_DOUBLE                       dTotalColorPotAmount;                   //彩池金额
};
//用户撤销发红包
struct CMD_S_RevokeHB
{
    CT_DWORD						dwUserID;							//用户id
    CT_WORD							wChairID;							//玩家id
    CT_WORD							wRevokeCount;						//要撤销数量
    CT_WORD							wRevokeSuccessCount;				//撤销成功数量
    CT_DWORD							dwRevokeHBID[MAX_FA_HB_COUNT];		//撤销红包的id
};
//广播用户申请批量发红包加入扫雷队列服务器回复
struct CMD_S_ApplyBatchFaHB
{
    CT_DWORD						dwUserID;							    //用户id
    CT_INT32                        iFaHBCount;                             //发红包成功数量
    CT_BYTE							cbIsFail;
    //申请是否失败 0成功，1，发红包的钱大于所拥有的钱，
    // 2，发红包的金额小于该场次的规定最小额，3发红包的金额大于该场次规定的最大额, 4中雷号错误，5，红包个数错吴, 6 当前发红包人数过多请稍后
};
//广播抢红包结算
struct CMD_S_GameEnd
{
	CMD_S_GameEnd():
			dwHBID(0),
			dwFaHBUserID(0),
			wCurrFaHBChairID(0),
			iFaHBTotalAmount(0),
			cbNoGrabHBCount(0),
			dBackTotalAmount(0.0),
			dGrabedTotalAmount(0.0),
			cbGrabedHBCount(0),
			dThunderBackTotalAmount(0.0),
			dRevenue(0.0),
            dWinColorPotAmount(0.0),
			dWinOrLose(0.0),
			dTotalWinLose(0.0),
            dTotalColorPotAmount(0.0)
	{
        memset(stGrabHBPlayer, 0, sizeof(stGrabHBPlayer));
	}
    CT_DWORD					dwHBID;                                      //红包id
	CT_DWORD					dwFaHBUserID;							    //用户id
    CT_WORD						wCurrFaHBChairID;					        //当前发红包玩家
    CT_INT32					iFaHBTotalAmount;					        //红包总金额
    CT_BYTE						cbNoGrabHBCount;							//没抢的红包数量
    CT_DOUBLE					dBackTotalAmount;							//退回的总金额
    CT_DOUBLE					dGrabedTotalAmount;							//被抢的总金额
    CT_BYTE					    cbGrabedHBCount;							//被抢红包数量
    CT_DOUBLE		            dThunderBackTotalAmount;					//中雷返现总金额
	CT_DOUBLE 					dRevenue;									//税收
    CT_DOUBLE                   dWinColorPotAmount;                         //获得彩池金额
	CT_DOUBLE 					dWinOrLose;									//本轮红包总输赢
	CT_DOUBLE           		dTotalWinLose;                    			//从进入到现在总输赢
    CT_DOUBLE                   dTotalColorPotAmount;                       //彩池金额
    GS_PlayerBaseData           stGrabHBPlayer[MAX_HB_COUNT];               //抢过红包的玩家
};
//获取红包被抢信息
struct CMD_S_GrabedHBInfo
{
    CT_DWORD						dwHBID;                              //红包id
    GS_PlayerBaseData               stGrabHBPlayer[MAX_HB_COUNT];               //抢过红包的玩家
};

//删除显示列表中的红包
struct CMD_S_DeleteDisplayHB
{
    CT_DWORD							dwHBID;                              //红包id
};
//发送正在显示的红包（每次发送多个此结构体）
struct CMD_S_SendDisplayHB
{
    GS_PlayerBaseData               stFaHBPlayer;                           //每个红包发红包玩家信息
	CT_WORD                         wLeftTime;                             //每个红包剩余时间
    GS_ApplyFaHBInfo                FaingHB;                                //每个红包正在显示的红包
    CT_BYTE                         cbSurplusHBCount;                       //每个红包剩余未被抢红包数量
    CT_BYTE                         cbIsSelfGrab;                           //是否自己抢过
};

//通知机器人抢红包
struct CMD_S_NoticeRobotGrabHB
{
    CT_WORD							wChairID;							    //发红包玩家id
    CT_DWORD						dwHBID;                                  //红包id
	CT_WORD							wGrabTime;						        //抢的时间
	CT_WORD							wTimeLeft;								//剩余时间
};
//通知机器人fa红包
struct CMD_S_NoticeRobotFaHB
{
    CT_WORD							wChairID;							    //发红包玩家id
    CT_INT32						iHBAllotCount;								//红包数量
};
//积分信息
struct tagScoreInfo
{
	CT_INT32						iScore;								//积分
	CT_BYTE							cbType;								//类型
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义
#define SUB_C_REVOKE_FA_HB					1									//用户撤销发红包
#define SUB_C_OPEN_HB						3									//用户打开红包请求
#define SUB_C_APPLY_BATCH_FA_HB 			4								    //用户申请批量发红包
#define SUB_C_GET_APPLY_LIST		        5									//获取申请发红包列表
#define SUB_C_GET_RANKING_LIST		        8									//获取在线玩家排行榜
#define SUB_C_GET_HB_GRABED_INFO		    15									//获取红包被抢信息
//用户打开红包请求
struct CMD_C_OpenHB : public MSG_GameMsgUpHead
{
    CT_WORD							wChairID;							//玩家id
    CT_DWORD						dwHBID;                              //红包id
};
//用户撤销发红包
struct CMD_C_RevokeHB : public MSG_GameMsgUpHead
{
    CT_WORD							wChairID;							//玩家id
    CT_WORD							wRevokeCount;						//撤销数量
    CT_DWORD						dwHBID[MAX_FA_HB_COUNT];				//红包id
};
//用户申请批量发红包
struct CMD_C_ApplyBatchFaHB : public MSG_GameMsgUpHead
{
    CT_WORD							wChairID;							//玩家id
    CT_BYTE							cbThunderNO;						//中雷号
    CT_INT32						iHBAmount;							//红包金额
    CT_INT32                        iFaHBCount;                         //发红包个数(发多少个红包)
    CT_INT32                        iHBAllotCount;                      //发的红包可以分配成多少个子包（包数）
};

//获取红包被抢信息(某红包被谁抢了，被谁抢了)
struct CMD_C_GrabedHBInfo : public MSG_GameMsgUpHead
{
    CT_DWORD							dwHBID;                              //红包id
};
//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

