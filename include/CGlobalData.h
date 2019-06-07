/******************************************************************
 Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
 File name:		CGlobalData.h
 Version:		1.0
 Description:    全局定义
 Author:			osc
 Create Date:	2012-02-14
 History:
 
 ******************************************************************/
#ifndef CGLOBALDATA_H_
#define CGLOBALDATA_H_
#include "CTType.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//无效椅子
#define		INVALID_CHAIR		65535

//服务名称长度
#define		SERVER_NAME_LEN	(32)

//账号长度
#define		ACCOUNTS_LEN	(64)

//web口令
#define		WEB_PWD_LEN		(128)

//昵称长度
#define		NICKNAME_LEN	(32)

//密码长度
#define		PASS_LEN		(33)

//头像URL长度
#define		HEAD_URL_LEN	(256)

//机器序列
#define		MACHINESERIAL_LEN (64)

//机器型号
#define		MACHINETYPE_LEN	(32)

//IP长度
#define		STRING_IP_LEN	  (32)

//手机号码长度
#define		MOBILE_NUM_LEN	(20)

//验证码长度
#define		VERIFY_CODE_LEN	(8)	

//Email长度
#define		EMAIL_LEN		(32)

//游戏种类名称(KindName)
#define		KINDNAME_LEN	(32)

//进程名称(ProcessName)
#define		PROCESSNAME_STR_LEN	(32)

//游戏房间名称
#define		GAMEROOM_NAME_LEN	(32)

//地址长度
#define     DWELLING_PLACE_LEN  (64)

//ip长度
#define     IP_LEN              (16)

//房间名字长度u
#define     ROOM_NAME_LEN       (64)

//UUID
#define		UUID_LEN			(33)

//所在地长度
#define		USER_LOCATE_LEN		(64)

//所在地长度2(针对IP)
#define		LOCATE_LEN2			(32)

//所在省份
#define 	PROVINCE_LEN		(16)

//文本加语音聊天长度
#define     GAME_VOICE_LEN		(256)

//时间长度
#define		TIME_LEN			(20)

//日期长度（不包括时间）eg.2016-12-26
#define		DATE_LEN			(12)

//时间长度（不包括日期）eg.10:57:01
#define		TIME_LEN_2			(10)

//时间区间 eg. 08:00-09:00
#define 	TIME_LEN_3			(12)

//私人房间参数
#define		PROOM_PARAM_LEN		(16)

//版本号长度
#define		VERSION_LEN			(10)

//邮件标题
#define		MAIL_TITLE			(64)

//邮件内容
#define		MAIL_CONTENT		(512)

//非百人游戏最大游戏人数
#define		MAX_PLAYER			5

//群组名称
#define		GROUP_NAME_LEN		32

//群组公告
#define		GROUP_NOTICE_LEN	302

//群组聊天
#define     GROUP_CHAT_LEN      300

//群组用户备注
#define     GROUP_REMARKS_LEN   32

//群组订单号
#define     GROUP_ORDER_LEN     20

//群组推广链接
#define     GROUP_PROMOTION_DOMIAN 64

//红包名称
#define		HONGBAO_NAME_LEN	32

//任务描述
#define		TASK_DESC_LEN		256

//系统消息长度
#define		SYSTEM_MSG_LEN		300

//真实名字长度
#define		REAL_NAME_LEN		30

//支付宝帐号长度
#define		ALIPAY_ACCOUNT_LEN	40

//银行卡长度
#define		BANK_CARD_LEN		21

//银行卡支行地址长度
#define		BANK_ADDRESS_LEN	60

//渠道主页长度
#define		CHANNEL_DOMAIN_LEN	32

//单位转发
#define		TO_DOUBLE			0.01
#define		TO_LL				100

//兑换收税
#define		EXCHANGE_REVENUE	0.02f

//一元夺宝标题
#define     DUOBAO_TITLE        32
//一元夺宝的描述
#define     DUOBAO_DES          256

//充值主页长度
#define		RECHARGE_DOMAIN_LEN				128
//新代理充值的Url长度
#define 	PROMOTER_RECHARGE_URL_LEN		128
//新代理充值的商户号
#define 	PROMOTER_RECHARGE_MERCHANT_LEN 	16
//新代理充值的token
#define		PROMOTER_RECHARGE_TOKEN_LEN		32

//新年红包一期内游戏房间数量
#define 	RED_PACKET_ROOM_COUNT			5
//红包的描述
#define 	RED_PACKET_DES					256
//红包大赢家数量
#define     RED_PACKET_BIGWINNER            3

//充值渠道数量
#define 	RECHARGE_COUNT					8

//聊天上传图片URL
#define 	CHAT_UPLOAD_URL					256

//公告的标题
#define     ANNOUNTCEMENT_TITLE_LEN         19
//公告的内容
#define     ANNOUNTCEMENT_CONTENT_LEN       2048

//充值订单长度
#define     RECHARGE_OREDER_LEN             25

//用户状态
enum USER_STATUS
{
    sGetOut = 0,		// 0,离开了
    sFree,				// 1,在房间站立
    sSit,				// 2,坐在椅子上，没按开始
    sReady,				// 3,同意游戏开始
    sPlaying,			// 4,正在玩
    sOffLine,			// 5,断线
    sLookOn,			// 6,旁观
    sGetOutAtPlaying    // 7,离开了还在游戏中.
};

//比赛状态
enum MATCH_STATUS
{
    sEnter = 0,         //进入比赛
    sJoin,              //参加比赛
    sMatch,             //比赛中
    sEnd,               //比赛结束
    sLeave,             //离开比赛
};

//登录消息头
struct MSG_LoginMsgHead
{
	CT_DWORD		dwMainID;							//主id
	CT_DWORD		dwSubID;							//子id
	CT_INT64		uValue1;							//多功能值1.client net addr
};

//游戏上行消息头
struct MSG_GameMsgUpHead
{
	CT_DWORD	dwUserID;					//玩家ID 
};

//游戏下行消息头
struct MSG_GameMsgDownHead
{
	CT_DWORD		dwMainID;	//主id
	CT_DWORD		dwSubID;	//子id
	CT_UINT64		uValue1;	//多功能值1.client net addr.
	CT_DWORD		dwValue2;	//多功能值2.close sock value.
};

//游戏逻辑消息头文件
struct CMD_GF_GameFrameHead
{
	CT_DWORD		dwMainID; //主id
	CT_DWORD		dwSubID;  //子id
};

//构造一下空的场景消息（给回放用）
struct MSG_GS_Free_GameScene
{
	CT_DWORD		dwCellScore;				//基础积分
	CT_WORD			wCurrPlayCount;				//当前局数
}; 


//宏定义
//内部消息是否验证.
#define		MSG_CRC32_G			(CT_FALSE)
//内部消息是否加密
#define		MSG_ENCRYPT_G		(CT_FALSE)
//memcached存储时间(默认3小时)
#define		MEMCACHED_EXPIRY_TIME		(10800)
#define     MEMCACHED_LOGIN_TIME        (1800)


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif



