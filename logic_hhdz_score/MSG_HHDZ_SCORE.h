#ifndef	__MESSAGE_HHDZ_SCORE_H_DEF__
#define	__MESSAGE_HHDZ_SCORE_H_DEF__

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

////////////////////////////////////////////////////////////////////////

#define GAME_PLAYER								200								//游戏人数

//数值定义
#define MAX_COUNT								3								//最大手牌张数
#define MAX_JETTON_NUM							5								//最大筹码个数
#define CARD_AREA_COUNT							2								//发牌区域
#define AREA_COUNT								3								//区域数目(红黑区域)
#define MAX_SIT_POS								6								//座位限制

#define HISTORY_COUNT							60								//历史记录
#define HISTORY_CARD_TYPE_COUNT					20								//历史牌型记录

#define HISTORY_ROW_COUNT					    32								//路单行数
#define HISTORY_COLS_COUNT					    6								//路单列数

#define USER_HISTORY_COUNT						20								//玩家游戏记录

#define CAN_JETTON_SCORE						3000							//至少需要块钱才能下注
//////////////////////////////////////////////////////////////////////////
//游戏状态
#define GAME_STATUS_FREE						100								//空闲状态（从来没有人进入过游戏的状态）
//#define GAME_STATUS_SENDCARD					101								//发牌状态
#define GAME_STATUS_SCORE						102								//下注状态
#define GAME_STATUS_END							103								//游戏结束

//场景消息
//#define SC_GAMESCENE_FREE						2001							//空闲场景消息
//#define SC_GAMESCENE_SENDCARD					2002							//发牌场景消息
#define SC_GAMESCENE_PLAY						2003							//游戏中场景消息
#define SC_GAMESCENE_END						2004							//结束场景消息

//空闲状态
//struct HHDZ_CMD_S_StatusFree
//{
//	//全局信息
//	CT_BYTE							cbTimeLeave;								//剩余时间
//};
//
//struct HHDZ_CMD_S_StatusSendCard
//{
//	//全局信息
//	CT_BYTE							cbTimeLeave;								//剩余时间
//	CT_BYTE							cbTotalTime;								//总时间
//};

enum enAreaId
{
	en_Hei = 0,
	en_Hong = 1,
	en_GoogLuck = 2,
};

enum enAreaWin
{
	en_HeiWin = 1,
	en_HongWin = 2,
};

enum enRecordPoint
{
    en_BluePoint = 1,	//蓝点
	en_HongPoint = 2,	//红点
};

//游戏状态
struct HHDZ_CMD_S_StatusPlay
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间

	//玩家信息
	CT_BYTE							cbCanAddLastBet;							//是否可以续注

	//全局下注
	CT_DOUBLE						dAllJettonScore[AREA_COUNT];				//全体总注(0: 黑区 1：红区 2：幸运区)

	//玩家下注
	CT_DOUBLE						dUserJettonScore[AREA_COUNT];				//个人每个区域总注(0: 黑区 1：红区 2：幸运区)
};

//结束状态
struct HHDZ_CMD_S_StatusEnd
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbTotalTime;								//总时间

	//全局下注
	CT_DOUBLE						dAllJettonScore[AREA_COUNT];				//全体总注(0: 黑区 1：红区 2：幸运区)
	//玩家下注
	CT_DOUBLE						dUserJettonScore[AREA_COUNT];				//个人每个区域总注(0: 黑区 1：红区 2：幸运区)

	//扑克信息
	CT_BYTE							cbTableCardArray[CARD_AREA_COUNT][MAX_COUNT];//桌面扑克(0: 黑区 1：红区 )
	CT_BYTE							cbTableCarType[CARD_AREA_COUNT];			 //桌面牌类型

	//玩家成绩
	CT_DOUBLE						dLWScore[8];								//玩家输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
	CT_BYTE							cbAreaWin[AREA_COUNT];						//区域输赢(1表示某个区域赢 0表示某个区域输)

	CT_DOUBLE						dUserAreaLWScore[8][AREA_COUNT];			//玩家各区域输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己) (0:庄家区域 1-4：下注区域 )

	CT_DOUBLE						dUserTotalScore[GAME_PLAYER];				//结算后玩家的总分
	CT_DWORD						dwUserID[GAME_PLAYER];						//玩家ID

	//结算信息
	CT_BYTE							cbBigWinnerHeadIndex;						//大赢家头像
	CT_BYTE							cbBigWinnerSex;								//大赢家性别
	CT_CHAR							szBigWinnerNickName[NICKNAME_LEN];			//大赢家昵称
	CT_DOUBLE						dBigWinnerScore;							//大赢家赢分
};


//////////////////////////////////////////////////////////////////////////
//服务器命令结构
//#define HHDZ_SUB_S_GAME_FREE						100							//游戏空闲，等待下注
//#define HHDZ_SUB_S_GAME_START						101							//游戏开始(发牌)
#define HHDZ_SUB_S_GAME_START_BET					102							//游戏开始下注
#define HHDZ_SUB_S_BET_SCORE_RESULT					103							//下注结果 
#define HHDZ_SUB_S_GAME_END							104							//游戏结束
#define HHDZ_SUB_S_BET_SCORE_RESULT_TIMED_REFRESH 	105							//定时刷新平台

#define HHDZ_SUB_S_USER_SIT_LIST					108							//座椅链表信息
#define HHDZ_SUB_S_OPERATE_FAIL						110							//操作失败
#define HHDZ_SUB_S_QUERY_HISTORY					111							//路单信息
#define HHDZ_SUB_S_ADD_LAST_BET						113							//续注[NN_CMD_S_LastBetResult]
#define HHDZ_SUB_S_QUERY_USER_RANK					114							//所有用户的下注排行[一组HHDZ_CMD_S_UserRank]

#define HHDZ_SUB_S_FOLLOW_RESULT					115							//发起跟投信息[HHDZ_CMD_S_FollowBet]
#define HHDZ_SUB_S_FOLLOW_INFO						116							//玩家跟注信息[HHDZ_CMD_S_FollowBetInfo]
#define HHDZ_SUB_S_SIT_BEFOLLOW_INFO				117							//被跟投的次数信息[HHDZ_CMD_S_BeFollowBetInfo]
#define HHDZ_SUB_S_CANCEL_FOLLOW					118							//取消跟投[无]

////游戏空闲，等待下注
//struct HHDZ_CMD_S_GameFree
//{
//	CT_BYTE							cbTimeLeave;								//剩余时间
//};
//
////游戏开始(发牌)
//struct HHDZ_CMD_S_GameStart
//{
//	CT_BYTE							cbTimeLeave;								//剩余时间
//};

//游戏开始下注
struct HHDZ_CMD_S_GameBet
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbCanAddLastBet;							//是否可以续注
};

//下注结果
struct HHDZ_CMD_S_BetResult
{
	CT_WORD							wChairID;									//下注玩家
	CT_BYTE							cbJettonArea;								//下注区域
	CT_WORD							wJettonIndex;								//下注索引
	CT_DOUBLE						dAreaBetScore[AREA_COUNT];					//各区域总已下注金额(0: 黑区 1：红区 2：幸运区)
	CT_DOUBLE						dUserBetScore[AREA_COUNT];					//玩家各区域已下注金额(0: 黑区 1：红区 2：幸运区)
};

//续注结果
struct HHDZ_CMD_S_LastBetResult
{
	CT_WORD							wChairID;									//下注玩家
	CT_BYTE							cbJettonArea;								//下注区域
	CT_DOUBLE						dJettonScore;								//下注金额
	CT_DOUBLE						dAreaBetScore[AREA_COUNT];					//各区域总已下注金额(0: 黑区 1：红区 2：幸运区)
	CT_DOUBLE						dUserBetScore[AREA_COUNT];					//玩家各区域已下注金额(0: 黑区 1：红区 2：幸运区)
};

//定时刷新下注结果
struct HHDZ_CMD_S_Timing_BetResult
{
	CT_DOUBLE						dAreaBetScore[AREA_COUNT];					//各区域本次下注金额(0: 黑区 1：红区 2：幸运区)
	CT_DOUBLE						dAreaTotalBetScore[AREA_COUNT];				//各区域总已下注金额(0: 黑区 1：红区 2：幸运区)
};

//游戏结束
struct HHDZ_CMD_S_GameEnd
{
	//下局信息
	CT_BYTE							cbTimeLeave;								//剩余时间

	//扑克信息
	CT_BYTE							cbTableCardArray[CARD_AREA_COUNT][MAX_COUNT];//桌面扑克
	CT_BYTE							cbTableCardType[CARD_AREA_COUNT];			//桌面牌类型

	//玩家成绩
	CT_DOUBLE						dLWScore[8];								//玩家输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
	CT_BYTE							cbAreaWin[AREA_COUNT];						//区域输赢(1表示某个区域赢 0表示某个区域输)
	CT_DOUBLE						dUserAreaLWScore[8][AREA_COUNT];			//玩家各区域输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己) (0: 黑区 1：红区 2：幸运区)
	CT_DOUBLE						dUserTotalScore[GAME_PLAYER];				//结算后玩家的总分
	CT_DWORD						dwUserID[GAME_PLAYER];						//玩家ID
	
	//结算信息
	CT_BYTE							cbBigWinnerHeadIndex;						//大赢家头像
	CT_BYTE							cbBigWinnerSex;								//大赢家性别
	CT_CHAR							szBigWinnerNickName[NICKNAME_LEN];			//大赢家昵称
	CT_DOUBLE						dBigWinnerScore;							//大赢家赢分

	//当局路单
	CT_BYTE                         cbHistory;									//本局区域输赢记录(1 黑赢 2 红赢)
	CT_BYTE							cbWinCardType;								//赢方牌型
	CT_BYTE							cbWinRate[CARD_AREA_COUNT];					//胜率
	
	//大路路单
	CT_BYTE							cbRemoveRow;								//是否删除一行
	CT_BYTE							cbPosRow;									//插入大路路单第几行
	CT_BYTE							cbPsoCol;									//插入大路路单第几列

	//大眼仔路
	CT_BYTE							cbRemoveBigEyeRow;							//是否删除一行大眼仔
	CT_BYTE							cbBigEyePosRow;								//插入大眼仔路第几行(255表示不用插入,下同)
	CT_BYTE							cbBigEyePosCol;								//插入大眼仔路第几列
    CT_BYTE                         cbBigEyeColor;                              //插入大眼仔路颜色

	//小路
	CT_BYTE							cbRemoveSmallRow;							//是否删除一行小路
	CT_BYTE							cbSmallPosRow;								//插入小路第几行
	CT_BYTE							cbSmallPosCol;								//插入小路第几列
    CT_BYTE                         cbSmallColor;                               //插入小路颜色

	//蟑螂路
	CT_BYTE							cbRemoveCockroachRow;						//是否删除一行蟑螂路
	CT_BYTE							cbCockroachPosRow;							//插入蟑螂路第几行
	CT_BYTE							cbCockroachPosCol;							//插入蟑螂路第几列
    CT_BYTE                         cbCockroachColor;                           //插入蟑螂路的颜色

	//黑方预测
	CT_BYTE							cbBlackPredictBigEye;						//大路仔路预测(255表示没有预测值，下同)
	CT_BYTE							cbBlackPredictSmall;						//小路预测
	CT_BYTE							cbBlackPredictCockroach;					//曱甴路预测

	//红方预测
	CT_BYTE							cbRedPredictBigEye;							//大路仔路预测
	CT_BYTE							cbRedPredictSmall;							//小路预测
	CT_BYTE							cbRedPredictCockroach;						//曱甴路预测

};

//座椅链表信息
struct HHDZ_CMD_S_UserSitList
{
	CT_DWORD						wSitList[MAX_SIT_POS];						//座椅信息
	CT_WORD							wBeFollowCount[MAX_SIT_POS];				//被跟投人数
};

//失败信息
struct HHDZ_CMD_S_ErrorCode
{
	CT_BYTE							cbLoseReason;								//失败原因
};

//历史记录信息
struct HHDZ_CMD_S_HistoryResult
{
	CT_BYTE                         cbHistory[HISTORY_COUNT];					//输赢记录(1黑赢，2红赢)
	CT_BYTE							cbWinCardType[HISTORY_CARD_TYPE_COUNT];		//赢方牌型
	CT_BYTE							cbCount;									//局数
	CT_BYTE							cbWinRate[CARD_AREA_COUNT];					//胜率	
	CT_BYTE							cbBigHistory[HISTORY_ROW_COUNT][HISTORY_COLS_COUNT]; 	    //大路路单
	CT_BYTE							cbBigEyeHistory[HISTORY_ROW_COUNT][HISTORY_COLS_COUNT];     //大眼仔路单
	CT_BYTE							cbSmallHistory[HISTORY_ROW_COUNT][HISTORY_COLS_COUNT]; 	    //小路单
	CT_BYTE							cbCockroachHistory[HISTORY_ROW_COUNT][HISTORY_COLS_COUNT]; 	//曱甴路单

	//黑方预测
	CT_BYTE							cbBlackPredictBigEye;						//大路仔路预测(255表示没有预测值，下同)
	CT_BYTE							cbBlackPredictSmall;						//小路预测
	CT_BYTE							cbBlackPredictCockroach;					//曱甴路预测
	//红方预测
	CT_BYTE							cbRedPredictBigEye;							//大路仔路预测
	CT_BYTE							cbRedPredictSmall;							//小路预测
	CT_BYTE							cbRedPredictCockroach;						//曱甴路预测
};

//所有用户下注排行
struct HHDZ_CMD_S_UserRank
{
	CT_DWORD						dwUserID;									//玩家ID
	CT_BYTE							cbgender;									//玩家性别
	CT_BYTE							cbHeadIndex;								//头像ID
	CT_BYTE							cbWinCount;									//赢的局数
	CT_DWORD						dwTotalJetton;								//下的总注（单位:元）
};

//跟投结果
struct HHDZ_CMD_S_FollowBet
{
	CT_BYTE							cbResult;									//(0: 成功 1: 你的金币不足 2: 跟投的位置没有玩家 3：你已经跟投了其他位置 4：不能跟投自己)
};

//跟投信息
struct HHDZ_CMD_S_FollowBetInfo
{
	CT_BYTE							cbBeFollowSitID;							//跟投的座椅ID(6个上座椅ID(0-5))
	CT_BYTE							cbFollowTimes;								//总跟投次数
	CT_BYTE							cbCurrentTimes;								//当前跟投次数
	CT_BYTE							cbFollowJettonIndex;						//跟投筹码索引
};

//被跟投信息
struct HHDZ_CMD_S_BeFollowBetInfo
{
	CT_BYTE							cbSitID;								   //跟投的座椅子ID(6个上座椅ID(0-5))
	CT_WORD							cbBeFollowTimes;						   //被跟投次数
};

//取消跟投
struct HHDZ_CMD_S_CancelFollowBet
{
	CT_BYTE							cbReason;									//取消跟投的原因(0: 个人取消 1: 金币不足被迫取消跟投 2: 达到跟投次数, 取消跟投 3: 座椅上没有玩家，取消跟投)
};

//////////////////////////////////////////////////////////////////////////
//客户端往服务器端发消息(C->S)
#define HHDZ_SUB_C_BET					1									//玩家下注
#define HHDZ_SUB_C_ADD_LAST_BET			2									//玩家续注【无协议体】
#define HHDZ_SUB_C_QUERY_USER_RANK		3									//玩家最近20局下注排行
#define HHDZ_SUB_C_FOLLOW_BET			4									//跟注[HHDZ_CMD_C_Follow_Bet]
#define HHDZ_SUB_C_CANCEL_FOLLOW		5									//取消跟注[无]

//下注
struct HHDZ_CMD_C_Bet : public MSG_GameMsgUpHead
{
	CT_BYTE							cbJettonArea;							//筹码区域
	CT_WORD							wJettonIndex;							//下注索引
};

//跟人家下注
struct HHDZ_CMD_C_Follow_Bet : public MSG_GameMsgUpHead
{
	CT_BYTE							cbSitID;								//跟投的座椅子ID(6个上座椅ID(0-5))
	CT_BYTE							cbJettonIndex;							//跟投的筹码索引
	CT_BYTE							cbCount;								//跟投次数
};

//////////////////////////////////////////////////////////////////////////////////
enum enHHDZ_ErrorCode
{
	ERROR_BANKE_NOT_BET     = 1,				//庄家不能下注
	ERROR_BET_SCORE		    = 2,				//下注筹码错误
	ERROR_BET_AREA		    = 3,				//下注区域错误
	ERROR_SCORE_NOT_ENOUGH  = 4,				//金币不足
	ERROR_SCORE_LIMIT		= 5,				//可下注金额已达上限
	ERROR_SIT_CHAIR_ID		= 6,				//椅子id错误

	ERROR_STATUS			= 7,				//当前状态下不允许操作
	ERROR_MESSAGE			= 8,				//消息错误

	ERROR_BET_TWO_AREA		= 9,				//不能同时下注红黑两个区域

	ERROR_OPERATE			= 20,				//操作错误
};
//////////////////////////////////////////////////////////////////////////////////

enum enHHDZ_CancelFollowCode
{
	en_Cancel_Self = 0,
	en_Cancel_NoMoney = 1,
	en_Cancel_AtTimes = 2,
	en_Cancel_SitNoPlayer = 3,
};

//////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

