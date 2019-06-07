#ifndef CMD_CONFISH_H_
#define CMD_CONFISH_H_
#include "CTType.h"
#include "CTData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//捕鱼相关的协议命令 
//登录命令
#define MDM_GR_LOGON				1									//登录信息
#define SUB_GR_LOGON_USERID			1									//I D 登录 
#define SUB_GR_LOGON_SUCCES         100									//登录成功
#define SUB_GR_LOGON_FAILURE        101									//登陆失败

#define LEN_MACHINE_ID				64									//手机标识
#define LEN_MACHINE_NAME            16									//昵称长度
#define LEN_PASSWORD				33									//密码长度

//用户动作
#define MDM_GR_USER					3									//用户信息
#define SUB_GR_USER_SITDOWN			2									//坐下请求
#define SUB_GR_USER_STANDUP			3									//起立请求
#define SUB_GR_TICK_USER			18									//剔除用户
#define SUB_GR_USER_OFFLINE			500									//用户离线

//用户状态
#define SUB_GR_USER_STATUS			102									//用户状态

//用户状态
#define US_NULL						0x00								//没有状态
#define US_FREE						0x01								//站立状态
#define US_SIT						0x02								//坐下状态
#define US_READY					0x03								//同意状态
#define US_LOOKON					0x04								//旁观状态
#define US_PLAYING					0x05								//游戏状态
#define US_OFFLINE					0x06								//断线状态

#define MDM_GF_FRAME				100									//框架命令

//////////////////////////////////////////////////////////////////////////////////

#define SUB_GF_GAME_OPTION			1									//游戏配置
#define SUB_GF_GAME_SCENE			101									//游戏场景
#define MDM_GF_GAME					200									//游戏命令



//////////////////////////////////////////////////////////////////////////////////
//分数同步消息
#define MDM_GR_REVERSECALL				10

#define SUB_GR_WRIRTESCORE_REVERSE		100								//反向写入用户金币
#define SUB_GP_WRIRTESCORE_SUCCESS		200								//捕鱼写分成功

struct CMD_GR_LogonUserID
{
	//登录信息
	CT_DWORD					dwUserID;								//用户 I D
	CT_DOUBLE                   lUserScore;								//玩家金币
	CT_WORD						wProductID;								//渠道编号
	CT_DWORD                    dwClientAddr;							//IP地址
	CT_WORD						cbClientKind[LEN_MACHINE_NAME];			//客户机型
	CT_DWORD                    dwSpreaderID;							//渠道标示
	CT_DOUBLE					lRechargeAccount;						//玩家累计充值
	CT_BYTE						reserved[2];							//保留字节
};

struct CMD_GR_LogonSuccess
{
	CT_DWORD                    dwUserID;								//用户编号
	CT_DOUBLE                   lUserScore;								//玩家积分
	CT_BYTE                     resv[20];								//保留字段
};

//登录失败
struct CMD_GR_LogonFailure
{
	CT_LONG							lErrorCode;							//错误代码
	CT_CHAR							szDescribeString[128];				//描述消息
};
//坐下
struct CMD_GR_UserSitDown
{
	CT_WORD							wTableID;							//桌子位置
	CT_WORD							wChairID;							//椅子位置
	//CT_WORD							szPassword[LEN_PASSWORD];			//桌子密码
};
//起立
struct CMD_GR_UserStandUp
{
	CT_WORD							wTableID;							//桌子位置
	CT_WORD							wChairID;							//椅子位置
	CT_BYTE							cbForceLeave;						//强行离开
};

//剔除用户
struct CMD_GR_TickUser
{
	CT_DWORD 						dwTargetUserID;						//用户ID
};

//用户状态
struct CMD_GR_UserStatus
{
	CT_DWORD						dwUserID;							//用户标识

	CT_WORD							wTableID;							//桌子索引
	CT_WORD							wChairID;							//椅子位置
	CT_BYTE							cbUserStatus;						//用户状态
};
struct CMD_GF_GameOption
{
	CT_BYTE							cbAllowLookon;						//旁观标志
	CT_DWORD						dwFrameVersion;						//框架版本	
	CT_DWORD						dwClientVersion;					//游戏版本
};

//写入用户积分
struct CMD_GR_WriteScoreReverse
{
	CT_DWORD						dwUserID;							//用户ID
	CT_DOUBLE						lUserScore;							//用户分数
	CT_DOUBLE 						lRelativeScore;						//用户相对分数
	CT_BYTE							bKeepConnect;						//保持连接
	CT_BYTE							resv[11];							//保留字段
};

//写分成功反馈
struct CMD_GP_WriteScoreReverseSuccess
{
	CT_DWORD 						dwUserID;							//用户ID
};

//用户离线
struct CMD_GR_UserOffline
{
	CT_DWORD						dwUserID;							//用户ID
};

//包装金蝉捕鱼的消息码
#define		MSG_FISH_GAME_MAIN									70
#define		SUB_C2S_FISH_GAME									7001
#define		SUB_S2C_FISH_GAME									7050

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif