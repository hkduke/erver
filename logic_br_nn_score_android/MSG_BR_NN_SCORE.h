#ifndef		___MESSAGE_BR_NN_SCORE_H_DEF___
#define	___MESSAGE_BR_NN_SCORE_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

////////////////////////////////////////////////////////////////////////

#define GAME_PLAYER								100								//游戏人数

//数值定义
#define MAX_COUNT								5								//最大手牌张数
#define FIRST_CARD								2								//首发牌张数字
#define MAX_JETTON_NUM							5								//最大筹码个数
#define AREA_COUNT								4								//区域数目(0:庄家区域 1-4：下注区域)
#define MAX_BANKER_COUNT						10								//最大坐庄次数
#define APPLY_BANKER_SCORE						2000000/*000*/					//申请庄家最低限制
#define APPLY_BANKER_SCORE_2					10000000						//10倍场申请庄家最低限制
#define SIT_SCORE_LIMIT							20000/*00*/						//坐下金币限制
#define MAX_SIT_POS								6								//座位限制
#define SYSTEM_BANKER_SCORE						500000000						//系统坐庄庄家金币
#define MAX_TYPE_MULTIPLE						5								//最大牌型倍数
#define MAX_TYPE_MULTIPLE2						10								//10倍场最大牌型倍数
#define BASE_CAIJINCHI_NUM						5000							//彩金池初始值

#define HISTORY_COUNT							20								//历史记录
#define USER_HISTORY_COUNT						20								//玩家游戏记录

//////////////////////////////////////////////////////////////////////////
//游戏状态
#define GAME_STATUS_FREE						100								//空闲状态
#define GAME_STATUS_SENDCARD					101								//发牌状态
#define GAME_STATUS_SCORE						102								//下注状态
#define GAME_STATUS_END							103								//游戏结束

//场景消息
#define SC_GAMESCENE_FREE						2001							//空闲场景消息
#define SC_GAMESCENE_SENDCARD					2002							//发牌场景消息
#define SC_GAMESCENE_PLAY						2003							//游戏中场景消息
#define SC_GAMESCENE_END						2004							//结束场景消息

//空闲状态
struct NN_CMD_S_StatusFree
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间

	//玩家信息
	CT_DOUBLE						dUserMaxScore;								//玩家还可下注金币限制
	
	//庄家信息
	CT_DWORD						wBankerUser;								//当前庄家
	CT_DOUBLE						dBankerWinScore;							//庄家成绩
	CT_DOUBLE						dBankerTotalWinScore;						//庄家总输赢
	CT_DOUBLE						dBankerScore;								//庄家分数
	CT_BYTE							cbBankCurrCount;							//庄家当前当庄次数
	CT_BYTE							cbBankTotalCount;							//庄家当庄最大次数

	//控制信息
	CT_DOUBLE						dApplyBankerCondition;						//申请庄家条件
	CT_DOUBLE						dAreaLimitScore;							//区域下注限制
	CT_DOUBLE						dUserSitLimitScore;							//玩家坐下金币限制
};

struct NN_CMD_S_StatusSendCard
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbTotalTime;								//总时间

	//玩家信息
	CT_DOUBLE						dUserMaxScore;								//玩家还可下注金币限制

	//庄家信息
	CT_DWORD						wBankerUser;								//当前庄家
	CT_DOUBLE						dBankerWinScore;							//庄家成绩
	CT_DOUBLE						dBankerTotalWinScore;						//庄家总输赢
	CT_DOUBLE						dBankerScore;								//庄家分数
	CT_BYTE							cbBankCurrCount;							//庄家当前当庄次数
	CT_BYTE							cbBankTotalCount;							//庄家当庄最大次数
	
	//控制信息
	CT_DOUBLE						dApplyBankerCondition;						//申请庄家条件
	CT_DOUBLE						dAreaLimitScore;							//区域下注限制

	//扑克信息
	CT_BYTE							cbTableCardArray[AREA_COUNT + 1][FIRST_CARD]; //首发桌面扑克(0:庄家区域 1-4：下注区域)
};

//游戏状态
struct NN_CMD_S_StatusPlay
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间
	//CT_DOUBLE						dStorageCount;								//库存数值

	//玩家信息
	CT_BYTE							cbCanAddLastBet;							//是否可以续注
	CT_DOUBLE						dUserMaxScore;								//玩家还可下注金币限制

	//庄家信息
	CT_DWORD						wBankerUser;								//当前庄家
	CT_DOUBLE						dBankerWinScore;							//庄家输赢分
	CT_DOUBLE						dBankerTotalWinScore;						//庄家总输赢
	CT_DOUBLE						dBankerScore;								//庄家分数
	CT_BYTE							cbBankCurrCount;							//庄家当前当庄次数
	CT_BYTE							cbBankTotalCount;							//庄家当庄最大次数

	//控制信息
	CT_DOUBLE						dApplyBankerCondition;						//申请庄家条件
	CT_DOUBLE						dUserSitLimitScore;						//玩家坐下金币限制

	//全局下注
	CT_DOUBLE						dAllJettonScore[AREA_COUNT + 1];			//全体总注(0:庄家区域 1-4：下注区域)

	//玩家下注
	CT_DOUBLE						dUserJettonScore[AREA_COUNT + 1];			//个人每个区域总注(0:庄家区域 1-4：下注区域)

	//扑克信息
	//CT_BYTE							cbTableCardArray[AREA_COUNT + 1][MAX_COUNT];//桌面扑克(0:庄家区域 1-4：下注区域)
	//CT_BYTE							cbTableCarType[AREA_COUNT + 1];				//桌面牌类型(0:庄家区域 1-4：下注区域)
	//扑克信息
	CT_BYTE							cbTableCardArray[AREA_COUNT + 1][FIRST_CARD]; //首发桌面扑克(0:庄家区域 1-4：下注区域)

	//玩家成绩
	CT_DOUBLE						dLWScore[8];								//玩家输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
	CT_DOUBLE						dAreaLWScore[AREA_COUNT + 1];				//区域输赢分(0:庄家区域 1-4：下注区域)
	//CT_DOUBLE						dRevenueScore;								//税收金额

	CT_WORD							wGameStatus;								//游戏状态
	CT_DOUBLE						dUserAreaLWScore[8][AREA_COUNT + 1];		//玩家各区域输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己) (0:庄家区域 1-4：下注区域 )
};

//结束状态
struct NN_CMD_S_StatusEnd
{
	//全局信息
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbTotalTime;								//总时间

	//庄家信息
	CT_DWORD						wBankerUser;								//当前庄家
	CT_DOUBLE						dBankerWinScore;							//庄家输赢分
	CT_DOUBLE						dBankerTotalWinScore;						//庄家总输赢
	CT_DOUBLE						dBankerPreScore;							//庄家结算前分数
	CT_DOUBLE						dBankerScore;								//庄家分数
	CT_BYTE							cbBankCurrCount;							//庄家当前当庄次数
	CT_BYTE							cbBankTotalCount;							//庄家当庄最大次数

	//控制信息
	CT_DOUBLE						dApplyBankerCondition;						//申请庄家条件
	CT_DOUBLE						dUserSitLimitScore;							//玩家坐下金币限制

	//全局下注
	CT_DOUBLE						dAllJettonScore[AREA_COUNT + 1];			//全体总注(0:庄家区域 1-4：下注区域)
	//玩家下注
	CT_DOUBLE						dUserJettonScore[AREA_COUNT + 1];			//个人每个区域总注(0:庄家区域 1-4：下注区域)

	//扑克信息
	CT_BYTE							cbHintCard[AREA_COUNT + 1][MAX_COUNT];		//牛牛提牌数据(标志为1表示提起)
	CT_BYTE							cbTableCardArray[AREA_COUNT + 1][MAX_COUNT];//桌面扑克(0:庄家区域 1-4：下注区域)
	CT_BYTE							cbTableCarType[AREA_COUNT + 1];				//桌面牌类型(0:庄家区域 1-4：下注区域)
	CT_INT32						iMultiple[AREA_COUNT + 1];					//倍数(0:庄家区域 1-4：下注区域)

	//玩家成绩
	CT_DOUBLE						dLWScore[8];								//玩家输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
	CT_DOUBLE						dAreaLWScore[AREA_COUNT + 1];				//区域输赢分(0:庄家区域 1-4：下注区域)
	CT_DOUBLE						dRevenueScore;								//税收金额
	CT_DOUBLE						dUserAreaLWScore[8][AREA_COUNT + 1];		//玩家各区域输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己) (0:庄家区域 1-4：下注区域 )
	CT_DOUBLE						dUserTotalScore[GAME_PLAYER];				//结算后玩家的总分
	CT_DWORD						dwUserID[GAME_PLAYER];						//玩家ID

	//结算信息
	CT_BYTE							cbEndType;									//结算类型(0 正常结束 1 通杀 2 通赔)
	CT_BYTE							cbBigWinnerHeadIndex;						//大赢家头像
	CT_BYTE							cbBigWinnerSex;								//大赢家性别
	CT_CHAR							szBigWinnerNickName[NICKNAME_LEN];			//大赢家昵称
	CT_DOUBLE						dBigWinnerScore;							//大赢家赢分

	//当局路单
	CT_BYTE                         cbHistory[4];								//本局区域输赢记录，0输，1赢
};


//////////////////////////////////////////////////////////////////////////
//服务器命令结构
#define NN_SUB_S_GAME_FREE							100							//游戏空闲，等待下注
#define NN_SUB_S_GAME_START							101							//游戏开始(发牌)
#define NN_SUB_S_GAME_START_BET						120							//游戏开始下注(120插入)
#define NN_SUB_S_BET_SCORE_RESULT					102							//下注结果 
#define NN_SUB_S_GAME_END							103							//游戏结束 
#define NN_SUB_S_BANKER_APPLY_OPERATE				104							//申请/取消申请庄家
#define NN_SUB_S_CHANGE_BANKER						105							//切换庄家
#define NN_SUB_S_APPLY_CANCEL_BANKER				106							//申请下庄
#define NN_SUB_S_SIT								107							//坐下/起来
#define NN_SUB_S_USER_SIT_LIST						108							//座椅链表信息
#define NN_SUB_S_BANKER_APPLY_LIST					109							//庄家申请链表
#define NN_SUB_S_OPERATE_FAIL						110							//操作失败
#define NN_SUB_S_QUERY_HISTORY						111							//10局区域输赢记录
#define NN_SUB_S_QUERY_CAIJINCHI					112							//查询上次彩金池获奖冠军
#define NN_SUB_S_ADD_LAST_BET						113							//续注[NN_CMD_S_LastBetResult]

#define NN_SUB_S_QUERY_USER_HISTORY					114							//所有用户的历史记录[一组NN_CMD_S_UserPlayHistory]
#define NN_SUB_S_FLASH_AREA							115							//

#define NN_SUB_S_PING								130							//测试ping值

enum GameEndType
{
	en_Normal_End		= 0,	//正常结束
	en_All_Kill			= 1,	//通杀
	en_All_Compensate	= 2,	//通赔
	en_No_Bet			= 3,	//没有下注(方便客户端不显示结算框)
};

//游戏空闲，等待下注
struct NN_CMD_S_GameFree
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	//CT_DOUBLE						dStorageCount;								//库存数值
	CT_DWORD						wBankerUser;								//庄家
	CT_DOUBLE						dBankerScore;								//庄家金币
	//CT_BYTE							cbBankCount;								//当庄次数
};

//游戏开始(发牌)
struct NN_CMD_S_GameStart
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbBankerCount;								//当前庄家当庄次数
	CT_DWORD						wBankerUser;								//庄家
	CT_DOUBLE						dBankerScore;								//庄家金币
	CT_BYTE							cbTableCardArray[AREA_COUNT+1][FIRST_CARD]; //首发桌面扑克(0:庄家区域 1-4：下注区域)
};

//特殊区域
struct NN_CMD_S_Game_FlashArea
{
	CT_BYTE							cbFlashArea[AREA_COUNT + 1];				//闪亮的区域
};

//游戏开始下注
struct NN_CMD_S_GameBet
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_BYTE							cbCanAddLastBet;							//是否可以续注
	CT_DOUBLE						dBetScoreLimit;								//下注金额限制
};

//下注结果
struct NN_CMD_S_BetResult
{
	CT_WORD							wChairID;									//下注玩家
	CT_BYTE							cbJettonArea;								//下注区域
	CT_WORD							wJettonIndex;								//下注索引
	CT_DOUBLE						dBankerLimitScore;							//庄家下注金额限制
	CT_DOUBLE						dUserLimitScore;							//玩家下注金额限制
	CT_DOUBLE						dAreaBetScore[AREA_COUNT + 1];				//各区域总已下注金额(0:庄家区域 1-4：下注区域)
	CT_DOUBLE						dUserBetScore[AREA_COUNT + 1];				//玩家各区域已下注金额(0:庄家区域 1-4：下注区域)
};

//续注结果
struct NN_CMD_S_LastBetResult
{
	CT_WORD							wChairID;									//下注玩家
	CT_BYTE							cbJettonArea;								//下注区域
	CT_DOUBLE						dJettonScore;								//下注金额
	CT_DOUBLE						dBankerLimitScore;							//庄家下注金额限制
	CT_DOUBLE						dUserLimitScore;							//玩家下注金额限制
	CT_DOUBLE						dAreaBetScore[AREA_COUNT + 1];				//各区域总已下注金额(0:庄家区域 1-4：下注区域)
	CT_DOUBLE						dUserBetScore[AREA_COUNT + 1];				//玩家各区域已下注金额(0:庄家区域 1-4：下注区域)
};

//游戏结束
struct NN_CMD_S_GameEnd
{
	//下局信息
	CT_BYTE							cbTimeLeave;								//剩余时间
	//CT_DOUBLE						dStorageCount;								//库存数值
	CT_DWORD						wBankerUser;								//庄家
	CT_DOUBLE						dBankerScore;								//庄家金币
	CT_DOUBLE						dBankerWinScore;							//庄家输赢分
	CT_DOUBLE						dBankerTotalWinScore;						//庄家总输赢分
	//CT_WORD							wBankerCount;								//当庄次数

	//扑克信息
	CT_BYTE							cbHintCard[AREA_COUNT + 1][MAX_COUNT];		//牛牛提牌数据(标志为1表示提起)
	CT_BYTE							cbTableCardArray[AREA_COUNT + 1][MAX_COUNT];//桌面扑克(0:庄家区域 1-4：下注区域)
	CT_BYTE							cbTableCarType[AREA_COUNT + 1];				//桌面牌类型(0:庄家区域 1-4：下注区域)
	CT_INT32						iMultiple[AREA_COUNT + 1];					//倍数(0:庄家区域 1-4：下注区域)

	//玩家成绩
	CT_DOUBLE						dLWScore[8];								//玩家输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
	CT_DOUBLE						dAreaLWScore[AREA_COUNT + 1];				//区域输赢分(0:庄家区域 1-4：下注区域)
	CT_DOUBLE						dRevenueScore;								//税收金额（弃用）
	CT_DOUBLE						dUserAreaLWScore[8][AREA_COUNT + 1];		//玩家各区域输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己) (0:庄家区域 1-4：下注区域 )
	CT_DOUBLE						dUserTotalScore[GAME_PLAYER];				//结算后玩家的总分
	CT_DWORD						dwUserID[GAME_PLAYER];						//玩家ID
	
	//结算信息
	CT_BYTE							cbEndType;									//结算类型(0 正常结束 1 通杀 2 通赔)
	CT_BYTE							cbBigWinnerHeadIndex;						//大赢家头像
	CT_BYTE							cbBigWinnerSex;								//大赢家性别
	CT_CHAR							szBigWinnerNickName[NICKNAME_LEN];			//大赢家昵称
	CT_DOUBLE						dBigWinnerScore;							//大赢家赢分

	CT_BYTE                         cbHistory[4];								//本局区域输赢记录，0输，1赢
	CT_BYTE							cbWinRate[4];								//10局胜率
	
	//CT_DOUBLE						dCaiJinChiRew[GAME_PLAYER];					//彩金池奖励金额
	//CT_WORD						wCaiJinChiPercent;							//彩金池奖励百分比		
};

//申请/取消申请庄家
struct NN_CMD_S_BankerApplyOperate
{
	CT_BYTE                         cbType;										//0:申请，1：取消
	CT_DWORD						wOperateUser;								//操作玩家
};

//切换庄家
struct NN_CMD_S_ChangeBanker
{
	CT_DWORD						wBankerUser;								//庄家
	CT_DOUBLE						dBankerScore;								//庄家金币
	CT_DWORD						wLastBankerUser;							//上一个庄家[65535表示系统庄家]
	CT_BYTE							cbReason;									//换庄原因[0: 系统庄家换给玩家(客户端不用关注) 1:满游戏局数 2:自己申请下庄 3: 金币不够]
};

//申请下庄
struct NN_CMD_S_CancelBanker
{
	CT_DWORD						wCancelUser;								//申请下庄
};

//坐下/起来结果
struct NN_CMD_S_SitResult
{
	CT_BYTE                         cbType;										//0:坐下，1：起来
	CT_DWORD						wOperateUser;								//坐/起玩家
	CT_BYTE							cbSitPos;									//位置
};

//座椅链表信息
struct NN_CMD_S_UserSitList
{
	CT_DWORD						wSitList[MAX_SIT_POS];						//座椅信息
};

//庄家申请列表
struct NN_CMD_S_Banker_Apply_List
{
	CT_DWORD						wBankerApplyList[GAME_PLAYER];				//庄家申请列表
	CT_DWORD						dwBankerApplyCount;					 		//申请人数
};

//失败信息
struct NN_CMD_S_ErrorCode
{
	CT_BYTE							cbLoseReason;								//失败原因
};

//历史记录信息
struct NN_CMD_S_HistoryResult
{
	CT_BYTE                         cbHistory[HISTORY_COUNT][4];				//十局区域输赢记录，0输，1赢
  	CT_BYTE							cbHistoryCardType[HISTORY_COUNT][4];		//下注区域牌型
	CT_BYTE							cbCount;									//局数
	CT_BYTE							cbWinRate[4];								//胜率					
};

//查询上次彩金池获奖冠军
struct NN_CMD_S_CaiJinChiResult
{
	CT_CHAR							szNickName[NICKNAME_LEN];					//冠军名字
	CT_CHAR							szHeadUrl[HEAD_URL_LEN];					//头像URL
	CT_BYTE							cbVipLevel;									//VIP等级
	CT_DOUBLE						dCaiJinChiRew;								//冠军获得奖励
	CT_DWORD						dwUserID;									//玩家id
};

//所有用户游戏局数记录
struct NN_CMD_S_UserPlayHistory
{
	CT_DWORD						dwUserID;									//玩家ID
	CT_BYTE							cbgender;									//玩家性别
	CT_BYTE							cbHeadIndex;								//头像ID
	CT_BYTE							cbWinCount;									//赢的局数
	CT_DWORD						dwTotalJetton;								//下的总注（单位:元）
};

//////////////////////////////////////////////////////////////////////////
//客户端往服务器端发消息(C->S)

#define NN_SUB_C_BET					1									//玩家下注
#define NN_SUB_C_BANKER_APPLY_OPERATE	2									//申请/取消申庄家
#define NN_SUB_C_APPLY_CANCEL_BANKER	3									//申请下庄
#define NN_SUB_C_SIT					4									//坐下
#define NN_SUB_C_ADD_LAST_BET			5									//玩家续注【无协议体】
#define NN_SUB_C_ADD_QUERY_USER_HISTORY 6									//玩家最近的游戏记录

#define NN_SUB_C_PING					10									//测试ping值

//下注
struct NN_CMD_C_Bet : public MSG_GameMsgUpHead
{
	CT_BYTE							cbJettonArea;							//筹码区域
	CT_WORD							wJettonIndex;							//下注索引
};

//申请/取消申请庄家
struct NN_CMD_C_BankerApplyOperate : public MSG_GameMsgUpHead
{
	CT_BYTE                         cbType;										//0:申请，1：取消
};

//申请下庄
struct NN_CMD_C_ApplyCancelBanker : public MSG_GameMsgUpHead
{

};

//坐下
struct NN_CMD_C_Sit : public MSG_GameMsgUpHead
{
	CT_BYTE                         cbType;							//0:坐下，1：起来
	CT_BYTE							cbSitPos;						//位置
};

//////////////////////////////////////////////////////////////////////////////////
enum enNNErrorCode
{
	ERROR_BANKE_NOT_BET     = 1,				//庄家不能下注
	ERROR_BET_SCORE		    = 2,				//下注筹码错误
	ERROR_BET_AREA		    = 3,				//下注区域错误
	ERROR_SCORE_NOT_ENOUGH  = 4,				//金币不足
	ERROR_SCORE_LIMIT		= 5,				//可下注金额已达上限
	ERROR_SIT_CHAIR_ID		= 6,				//椅子id错误

	ERROR_APPLY_LIMIT		= 7,				//申请庄家条件不足
	ERROR_IS_BANKER			= 8,				//已做庄
	ERROR_IS_APPLY			= 9,				//已申请

	ERROR_NOT_APPLY			= 10,				//未申请
	ERROR_NOT_BANKER		= 11,				//不是庄家
	ERROR_SIT_LIMIT			= 12,				//坐下条件不足
	ERROR_SIT_NOT_EMPTY		= 13,				//该座位已有人
	ERROR_OPERATE			= 14,				//操作错误

	ERROR_STATUS			= 15,				//当前状态下不允许操作
	ERROR_MESSAGE			= 16,				//消息错误
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

