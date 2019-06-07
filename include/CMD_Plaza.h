/******************************************************************
 Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
 File name:		CMD_Plaza.h
 Version:		1.0
 Description:    大厅指令,外部消息定义
 Author:			osc
 Create Date:	2014-04-25
 History:
 
 ******************************************************************/

#ifndef CMD_PLAZA_HEAD_FILE
#define CMD_PLAZA_HEAD_FILE

#include "CTType.h"
#include "CGlobalData.h"
#include <regex>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

/////////////////////////////////////////////////////////////////////////////////////
//客户端发给服务器的的登录消息
#define		MSG_LOGIN_MAIN					6		
#define		SUB_CS_ACCOUNTS_LOGIN			601			//帐号登录【MSG_CS_LoginData】
#define		SUB_CS_ACCOUNTS_REGISTER		602			//帐号注册【MSG_CS_LoginData】
#define		SUB_CS_ACCOUNTS_LOGIN_REGISTER  603			//登录或者注册【MSG_CS_LoginData】
#define		SUB_CS_ACCUNNTS_SIGN_IN		    604			//用户签到【MSG_CS_SignIn】
#define		SUB_CS_PULL_MAIL				605			//用户拉取邮件【MSG_CS_PullMail】
#define		SUB_CS_READ_MAIL				606			//查阅邮件【MSG_CS_ReadMail】
#define		SUB_CS_GET_BENEFIT				607			//获取救济金【MSG_CS_GetBenefit】
#define		SUB_CS_EXCHANGE_SCORE			608			//钻石兑换金币【MSG_CS_ExchangeScore】
#define		SUB_CS_BIND_PROMOTER			609			//绑定代理ID【MSG_CS_Bind_Promoter】
#define		SUB_CS_BIND_MOBILE				610			//绑定帐号【MSG_CS_Bind_Mobile】
#define		SUB_CS_PAY_SCORE_BY_APPLY		611			//苹果充值【MSG_CS_Pay_UserScore_ByApple】
#define		SUB_CS_SET_BANK_PWD				612			//设置银行密码【MSG_CS_Set_BankPwd】
#define		SUB_CS_MODIFY_BANK_PWD			613			//修改银行密码【MSG_CS_Modify_BankPwd】
#define		SUB_CS_SAVE_SCORE_TO_BANK		614			//银行存款【MSG_CS_SaveScore_To_Bank】
#define		SUB_CS_TAKE_SCORE_FROM_BANK		615			//银行取款【MSG_CS_TakeScore_From_Bank】
#define		SUB_CS_GET_VERIFICATION_CODE	616			//获取手机验证码【MSG_CS_Get_VerificationCode】
#define		SUB_CS_RESET_PWD				617			//重置密码【MSG_CS_Ret_Pwd】
#define		SUB_CS_MODIFY_PERSONAL_INFO		618			//修改个人资料【MSG_CS_Modify_PersonalInfo】
#define		SUB_CS_DELETE_MAIL				619			//删除邮件【MSG_CS_DelMail】
#define		SUB_CS_BIND_ALIPAY_ACCOUNT		620			//绑定支付宝帐号【MSG_CS_Bind_AliPay】
#define		SUB_CS_BIND_BANK_ACCOUNT		621			//绑定银行卡帐号【MSG_CS_Bind_BankCard】
#define		SUB_CS_EXCHANGE_SCORE_TO_RMB	622			//兑换金币【MSG_CS_Exchange_Score_To_Rmb】
#define		SUB_CS_EXCHANGE_RMB_RECORD		623			//查询兑换记录【MSG_CS_Exchange_RMB_Record】
#define		SUB_CS_UNRED_EXCHANGE			624			//兑换按钮不显示红点【MSG_CS_UnRed_Exchange】
#define 	SUB_CS_REPORT_USER				625			//举报玩家【MSG_CS_Report_User_ForPlaza】
#define 	SUB_CS_QUERY_BENEFIT			626			//查询救济金【MSG_CS_QueryBenefit】
#define 	SUB_CS_QUERY_RECHARGE_RECORD	627			//查询充值记录【MSG_CS_Query_RechargeRecord】

#define		SUB_CS_ANDROID_CONNECT			630			//机器连接【MSG_CS_Android_Connect】

#define		SUB_SC_LOGIN_SUCCESS			650			//登录成功【MSG_SC_LoginSuccess】
#define		SUB_SC_LOGIN_FAILED				651			//登录失败【MSG_SC_LoginError】

#define		SUB_SC_GAME_KIND				652			//游戏列表【MSG_SC_GameItem】
#define		SUB_SC_ROOM_KIND				653			//对应的房间列表【MSG_SC_RoomKindItem】
#define		SUB_SC_PRIVATE_ROOM_INFO		654			//私人房信息【MSG_SC_PrivateRoomInfo】
#define		SUB_SC_SELF_PROOM_INFO			655			//个人私人房间信息【MSG_SC_SelfPrivateRoomInfo】
#define		SUB_SC_ONLINE_INFO				656			//玩家在线信息【MSG_SC_OnlineInfo】
#define		SUB_SC_LONG_DISTANCE_LOGIN		657			//玩家异地登录【MSG_SC_LongDistance_Login】
#define		SUB_SC_SIGN_IN_INFO				658			//玩家签到信息【MSG_SC_SignIn_Info】
#define		SUB_SC_SIGN_IN_RESULT			659			//玩家签到结果【MSG_SC_SignIn_Result】
#define		SUB_SC_MAIL_INFO				660			//玩家的邮件【MSG_SC_Mail_Info】
#define		SUB_SC_MAIL_INFO_FINISH			661			//玩家的邮件【无】
#define		SUB_SC_READ_MAIL_RESULT			662			//玩家阅读邮件结果【MSG_SC_ReadMailResult】

#define		SUB_SC_WELFARE_INFO				663			//玩家福利信息(签到救济金等)【MSG_SC_Welfare_Info】
#define		SUB_SC_GET_BENEFIT_RESULT		664			//玩家领取救济金结果【MSG_SC_GetBenefit_Result】
#define		SUB_SC_EXCHANGE_SCORE_RESULT	665			//兑换结果【MSG_SC_ExchangeScore_Result】 
#define		SUB_SC_BASE_SCORE_INFO			666			//玩家积分相关信息【MSG_SC_UserBaseScore】
#define		SUB_SC_BIND_PROMOTER			667			//绑定代理ID【MSG_SC_Bind_Promoter】
#define		SUB_SC_BIND_MOBILE				668			//绑定帐号【MSG_SC_Bind_Mobile】
#define		SUB_SC_SET_BANK_PWD				669			//设置银行密码返回【MSG_SC_Set_BankPwd】
#define		SUB_SC_MODIFY_BANK_PWD			670			//修改银行密码返回【MSG_SC_Modify_BankPwd】
#define		SUB_SC_SAVE_SCORE_TO_BANK		671			//银行存款返回【MSG_SC_Save_Score_To_Bank】
#define		SUB_SC_TAKE_SCORE_FROM_BANK		672			//银行取款返回【MSG_SC_Take_Score_From_Bank】
#define		SUB_SC_UPDATE_BANK_SCORE		673			//更新银行金币【MSG_SC_Update_BankScore】
#define		SUB_SC_VERIFICATION_CODE		674			//获取验证码返回【MSG_SC_VerificationCode】
#define		SUB_SC_RESET_PWD				675			//重置密码返回【MSG_SC_Reset_Pwd】
#define		SUB_SC_MODIFY_PERSONAL_INFO		676			//修改个人资料【MSG_SC_Modify_PersonalInfo】
#define		SUB_SC_DELETE_MAIL				677			//删除邮件返回【MSG_SC_DeleteMail】
#define		SUB_SC_BIND_EXCHANGE_ACCOUNT	678			//绑定兑换帐号返回【MSG_SC_Bind_ExchangeAccount】
#define		SUB_SC_EXCHANGE_RMB_RESULT		679			//兑换结果【MSG_SC_Exchange_To_Rmb】
#define		SUB_SC_EXCHANGE_RMB_RECORD		680			//兑换结果【一组MSG_SC_Exchange_Rmb_Record】
#define		SUB_SC_BIND_MOBILE_ALREADY		681			//游客登录已经绑定帐号【MSG_SC_BindMobile_Already】
#define 	SUB_SC_BIND_MOBILE_SCORE		682			//绑定手机赠送金币【MSG_SC_BindMobile_PresentScore】
#define 	SUB_SC_REPORT_USER_FORPLAZA		683			//举报玩家【MSG_SC_Report_User_ForPlaza】
#define     SUB_SC_QUERY_BENEFIT            684         //查询救济金【MSG_SC_QueryBenefit】
#define 	SUB_SC_RECHARGE_RECORD			685			//充值记录【一组MSG_SC_RechargeRecord,暂定最多查20条】

#define		SUB_SC_ANDROID_CONNECT			690			//机器连接返回【MSG_SC_AndroidConnect】

#define		SUB_SC_GAME_VERSION				695			//游戏版本号列表返回【一组MSG_SC_GameVersion】

/*struct MSG_CS_Register
{
	CT_CHAR								szAccount[ACCOUNTS_LEN];			//登录账号
	CT_CHAR                             szPassword[PASS_LEN];				//登录密码(MD5密文)
	CT_CHAR								szMachineSerial[MACHINESERIAL_LEN];	//手机序列号芯片ID,必须唯一.
	CT_CHAR								szMachineType[MACHINETYPE_LEN];		//手机型号
};

struct MSG_CS_RegisterEx : public MSG_CS_Register
{
	CT_CHAR								szClientIP[STRING_IP_LEN];			//客户端IP
	CT_UINT64							uClientSock;						//客户端sock地址
};*/

//登录数据
struct MSG_CS_LoginData
{
	CT_CHAR								szAccount[ACCOUNTS_LEN];			//登录账号
	CT_CHAR                             szPassword[PASS_LEN];				//登录密码(MD5密文)
	CT_CHAR								szMachineSerial[MACHINESERIAL_LEN];	//手机序列号芯片ID,必须唯一.
	CT_CHAR								szMachineType[MACHINETYPE_LEN];		//手机型号
	CT_CHAR								szLocation[USER_LOCATE_LEN];		//所在地
	CT_CHAR                             szProvince[PROVINCE_LEN];           //省份
	CT_DWORD							dwVerificationCode;					//验证码
	CT_BYTE								cbPlatformID;						//平台ID
	CT_BYTE                             cbIsGetChannelID;                   //是否拿到渠道ID
	CT_DWORD							dwChannelID;						//渠道ID
	CT_DWORD 							dwUserID;                           //用户ID
	CT_DWORD                            dwGroupUserID;                      //上级群组ID
};

struct MSG_CS_LoginDataEx : public MSG_CS_LoginData
{
	CT_CHAR								szClientIP[STRING_IP_LEN];			//客户端IP
	CT_UINT64							uClientSock;						//客户端sock地址
};

//用户签到
struct MSG_CS_SignIn
{
	CT_DWORD	dwUserID;							// 用户ID
};

//用户签到
struct MSG_CS_SignInEx : public MSG_CS_SignIn
{
	CT_UINT64			uClientSock;						//客户端sock地址
};

//用户领取救济金
struct MSG_CS_GetBenefit
{
	CT_DWORD	dwUserID;				//用户ID
};

//用户领取救济金
struct MSG_CS_GetBenefitEx : public MSG_CS_GetBenefit
{
	CT_UINT64			uClientSock;	//客户端地址
};

//用户查询救济金
struct MSG_CS_QueryBenefit
{
    CT_DWORD    dwUserID;               //查询救济金
};

//用户查询充值记录
struct MSG_CS_Query_RechargeRecord
{
	CT_DWORD 	dwUserID;				//查询充值
};

//用户兑换金币
struct MSG_CS_ExchangeScore
{
	CT_DWORD	dwUserID;				//用户ID
	CT_WORD		wGem;					//兑换钻石
};

//用户领取救济金
struct MSG_CS_ExchangeScoreEx : public MSG_CS_ExchangeScore
{
	CT_UINT64			uClientSock;	//客户端地址
};

//用户绑定代理
struct MSG_CS_Bind_Promoter
{
	CT_DWORD	dwUserID;				//用户ID
	CT_DWORD	dwPromoterID;			//代理ID
};

//绑定手机号码
struct MSG_CS_Bind_Mobile
{
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szMobileNum[MOBILE_NUM_LEN];		//手机号
	CT_CHAR		szPassword[PASS_LEN];				//密码
	CT_DWORD	dwVerifyCode;						//验证码
}; 

//获取手机验证码
struct MSG_CS_Get_VerificationCode
{
	CT_BYTE		cbType;								//验证码类型 (1:注册或绑定 2:重置密码 3:修改支付宝)
	CT_BYTE     cbPlatformId;                       //平台ID
	CT_CHAR		szMobileNum[MOBILE_NUM_LEN];		//手机号码
}; 

struct MSG_CS_Get_VerificationCodeEx : public MSG_CS_Get_VerificationCode
{
	CT_UINT64	uClientSock;	//客户端地址
};

//重置密码
struct  MSG_CS_Reset_Pwd
{
	CT_CHAR		szMobileNum[MOBILE_NUM_LEN];		//手机号码
	CT_CHAR		szPassword[PASS_LEN];				//密码
	CT_DWORD	dwVerifyCode;						//验证码
	CT_BYTE 	cbPlatformId;						//平台ID
};

//重置密码
struct  MSG_CS_Reset_PwdEx : public MSG_CS_Reset_Pwd
{
	CT_UINT64	uClientSock;	//客户端地址
};

//外挂机器登录
struct MSG_CS_Android_Connect
{
	CT_CHAR		szMachineType[MACHINETYPE_LEN];	//机器标识
};

//用户拉取邮件
struct MSG_CS_PullMail
{
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szUUID[UUID_LEN];					//session.
};

//用户拉取邮件
struct MSG_CS_PullMailEx : public MSG_CS_PullMail
{
	CT_UINT64		uClientSock;					//客户端sock地址
};

//用户阅读邮件
struct MSG_CS_ReadMail
{
	CT_DWORD	dwMailID;							//邮件ID
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szUUID[UUID_LEN];					//session.
};

//用户阅读邮件
struct MSG_CS_ReadMailEx : public MSG_CS_ReadMail
{
	CT_UINT64		uClientSock;					//客户端sock地址
};

//用户删除邮件
struct MSG_CS_DelMail
{
	CT_DWORD	dwUserID;							//用户ID
	CT_DWORD	dwMailID;							//邮件ID
};

//苹果测试充值
struct MSG_CS_Pay_UserScore_ByApple
{
	CT_DWORD		dwUserID;						//用户ID
	CT_DWORD		dwAddScore;						//增加金币
};

//设置银行密码
struct MSG_CS_Set_BankPwd
{
	CT_DWORD		dwUserID;						//用户ID
	CT_CHAR			szBankPwd[PASS_LEN];			//银行密码
};

//修改银行密码
struct MSG_CS_Modify_BankPwd
{
	CT_DWORD		dwUserID;						//用户ID
	CT_CHAR			szOldBankPwd[PASS_LEN];			//旧密码
	CT_CHAR			szNewBankPwd[PASS_LEN];			//新密码
};

//银行存款
struct MSG_CS_SaveScore_To_Bank
{
	CT_DWORD		dwUserID;						//用户ID
	CT_DOUBLE		dSaveScore;						//存款金币
};

//银行取款
struct MSG_CS_TakeScore_From_Bank
{
	CT_DWORD		dwUserID;						//用户ID
	CT_DOUBLE		dTakeScore;						//取金币数
	CT_CHAR			szBankPwd[PASS_LEN];			//银行密码
};

//修改个人资料
struct MSG_CS_Modify_PersonalInfo
{
	CT_DWORD		dwUserID;						//用户ID
	CT_BYTE			cbHeadId;						//头像ID
	CT_BYTE			cbSex;							//性别
};

//绑定支付宝帐号
struct MSG_CS_Bind_AliPay
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbType;								//类型(1: 初次绑定 2：修改绑定)
	CT_CHAR		szAliPayAccount[ALIPAY_ACCOUNT_LEN];//支付宝帐号
	CT_CHAR		szRealName[REAL_NAME_LEN];			//真实姓名
	CT_DWORD	dwVerifyCode;						//验证码
};

//绑定银行卡帐号
struct MSG_CS_Bind_BankCard
{
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szBankCard[BANK_CARD_LEN];			//银行卡号
	CT_CHAR		szRealName[REAL_NAME_LEN];			//真实姓名
	CT_CHAR		szBankAddress[BANK_ADDRESS_LEN];	//支行地址
	CT_BYTE		cbBankCardType;						//银行卡类型[1.中国银行 2.工商银行 3.建设银行 4.农业银行 5.招商银行 6.中信银行 7.交通银行 8.民生银行 9.华夏银行 10.平安银行]
};

//兑换RMB
struct MSG_CS_Exchange_Score_To_Rmb
{
	CT_DWORD	dwUserID;							//用户ID
	CT_DWORD	dwExchangeScore;					//兑换金币额
	CT_BYTE		cbType;								//类型（2. 支付宝 3. 网银）
};

//查询兑换记录
struct MSG_CS_Exchange_RMB_Record
{
	CT_DWORD	dwUserID;							//用户ID
};

//兑换按钮不显示红点
struct MSG_CS_UnRed_Exchange
{
	CT_DWORD	dwUserID;
};

//举报玩家
struct MSG_CS_Report_User_ForPlaza
{
	CT_DWORD 	dwUserID;							//玩家ID
	CT_DWORD 	dwBeReportUserID[4];				//被举报玩家
	CT_DWORD 	dwDrawID;							//记录ID
	CT_WORD 	wGameID;							//游戏ID
	CT_WORD 	wKindID;        					//类型ID
	CT_CHAR 	szDate[DATE_LEN];					//记录日期
};

//登陆失败
struct MSG_SC_LoginError : public MSG_GameMsgDownHead
{
	CT_INT32							lErrorCode;		//错误代码
};

//登陆成功
struct MSG_SC_LoginSuccess : public MSG_GameMsgDownHead
{
	CT_DWORD							dwUserID;						//用户ID
	CT_BYTE								cbOpenBank;						//开通银行(0 未开通 1 已开通)
	CT_BYTE								cbBindMobile;					//绑定手机(0 未绑定 1 已绑定)
	CT_BYTE								cbShowExchange;					//是否显示兑换
	CT_BYTE								cbGender;						//性别
	CT_BYTE								cbHeadId;						//头像id
	CT_BYTE								cbVipLevel;						//VIP等级
	CT_BYTE								cbVipLevel2;					//VIP等级2
	CT_BYTE								cbNextVip2;						//下一个等级2
	CT_DWORD							cbNextVip2NeedRecharge;			//下一个等级2需要的充值额
	CT_DWORD							dwRecharge;						//用户总充值
	CT_CHAR								szNickName[NICKNAME_LEN];		//昵称
	CT_CHAR								szAliPayAccout[ALIPAY_ACCOUNT_LEN];	//支付宝帐号
	CT_CHAR								szAliPayName[REAL_NAME_LEN];		//支付宝姓名
	CT_CHAR								szBankCardNum[BANK_CARD_LEN];		//银行卡号
	CT_CHAR								szBankCardName[REAL_NAME_LEN];		//银行卡姓名
	//CT_CHAR								szUUID[UUID_LEN];				//session.
	CT_DOUBLE							dScore;							//金币
	CT_DOUBLE							dBankScore;						//银行金币
	CT_CHAR								szWebPwd[WEB_PWD_LEN];			//web口令
	CT_BYTE								cbNewAccount;					//新帐号
	CT_BYTE                             cbPlatformId;                   //平台ID
};

struct MSG_SC_LoginSuccessTest
{
	CT_DWORD							dwUserID;						//用户 I D
	CT_DWORD							dwGem;							//宝石
	CT_BYTE								cbGender;						//性别
	CT_BYTE								cbPlatformID;					//平台ID
	CT_CHAR								szNickName[NICKNAME_LEN];		//昵称
	CT_CHAR								szHeadUrl[HEAD_URL_LEN];		//头像
	CT_CHAR								szUUID[UUID_LEN];				//session.
};

//游戏列表
struct MSG_SC_GameItem
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wSortID;
	CT_CHAR			szKindName[KINDNAME_LEN];
};

//游戏房间类型
struct MSG_SC_RoomKindItem
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wRoomKindID;
	CT_WORD			wSortID;
	CT_BOOL			bHasServer;
	CT_DWORD		dwCellScore;
	CT_WORD			wStartMaxPlayer;
	CT_DWORD		dwEnterMinScore;
	CT_DWORD		dwEnterMaxScore;
};

//开房桌信息
struct MSG_SC_PrivateRoomInfo
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_CHAR			szJsonParam[PROOM_PARAM_LEN];
};

//个人的开房桌信息
struct MSG_SC_SelfPrivateRoomInfo : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_DWORD		dwRoomNum;
};

//玩家的在线信息
struct MSG_SC_OnlineInfo : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;							// 游戏ID
	CT_WORD			wKindID;							// 游戏子类型
	CT_WORD			wRoomKindID;						// 房间类型
	CT_DWORD		dwRoomNumOrServerID;				// 私人房ID或者金币房serverid
};

//玩家在异地登录
struct MSG_SC_LongDistance_Login
{
	CT_DWORD		dwUserID;							// 玩家在异地登录
};

//玩家签到信息
struct MSG_SC_SignIn_Info : public MSG_GameMsgDownHead
{
	CT_BYTE			cbSignInDays;						// 连续签到天数
	CT_BYTE			cbTodaySignIn;						// 今天是否已经签到(0：今天未签到 1：今天已经签到)
};

//玩家签到
struct MSG_SC_SignIn_Result : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			// 签到结果(0: 签到成功, 1: 今天已经签到)
	CT_WORD			wSignInDays;		// 连续签到天数
	CT_WORD			wRewardGem;			// 奖励钻石
	CT_DWORD		dwRewardScore;		// 奖励金币
};

//玩家邮件
struct MSG_SC_Mail_Info// : public MSG_GameMsgDownHead
{
	CT_DWORD	dwMailID;					//邮件ID
	CT_DWORD	dwUserID;					//用户ID（为0时表示是系统邮件）
	CT_BYTE		cbMailType;					//邮件类型(1:表示阅后即毁普通邮件, 2:表示阅后不毁普通邮件)
	CT_BYTE		cbState;					//邮件状态(0:表示未读, 1:表示已读, 2:登录弹框)
	CT_CHAR		szTitle[MAIL_TITLE];		//邮件标题
	CT_CHAR		szContent[MAIL_CONTENT];	//邮件内容
	//CT_WORD		wGem;						//奖励钻石（如果没有奖励为0）
	CT_DOUBLE	dScore;						//奖励积分
	CT_CHAR		szTime[TIME_LEN];			//邮件时间
};

struct MSG_SC_Mail_InfoEx : public MSG_SC_Mail_Info
{
	CT_BYTE 	cbScoreType;                //金币变化类型
};

//玩家阅读邮件
struct MSG_SC_ReadMailResult : public MSG_GameMsgDownHead
{
	CT_BYTE		cbResult;		// 阅读结果(0: 成功, 1: 没有此邮件， 2: 邮件已经读取过了)
	CT_DWORD	dwMailID;		// 邮件ID
	CT_DOUBLE	dRewardScore;	// 附件奖励金币
	CT_DOUBLE	dTotalScore;	// 总金币
};

//玩家删除邮件
struct MSG_SC_DeleteMail : public MSG_GameMsgDownHead
{
	CT_BYTE		cbResult;		// 删除结果(0: 成功 1: 没有找到这个邮件)
	CT_DWORD	dwMailID;		// 邮件ID
};

//玩家救济金信息
struct MSG_SC_Welfare_Info : public MSG_GameMsgDownHead
{
	CT_WORD			wSignInDays;			// 连续签到天数
	CT_BYTE			cbTodaySignIn;			// 今天是否已经签到(0：今天未签到 1：今天已经签到)

	CT_BYTE			cbBenefitCount;			// 救济金已经领取次数
	CT_BYTE			cbBenefitTotalCount;	// 救济金总共能领取次数
	CT_DWORD		dwRewardScore;			// 每次领取的金币
	CT_DWORD		dwLessScore;			// 低于这个值可以领取救济金
};

//玩家领取救济金
struct MSG_SC_GetBenefit_Result : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			// 领取结果(0: 领取成功, 1: 今日领取已经达到最大次数限制，2: 金币高于领取救济金条件 3: 在游戏中不能领取救济金)
};

//查询玩家救济金信息返回
struct MSG_SC_QueryBenefit : public MSG_GameMsgDownHead
{
	CT_BYTE			cbCanGetBenefit;        // 是否能领取救济金(0不能领取，1能领取)
    CT_BYTE			cbBenefitTotalCount;	// 救济金总共能领取次数
    CT_BYTE			cbBenefitLeftCount;		// 救济金已经领取次数
    CT_DOUBLE 		dLessScore;				// 低于多少钱
    CT_DOUBLE	    dRewardScore;			// 当前能领取的金币
};

//查询充值记录返回
struct MSG_SC_RechargeRecord
{
	CT_BYTE			cbType;					        // 充值类型(1:微信 2:支付宝 3:网银 4:QQ钱包 5:代理 6:银闪付)
    CT_BYTE         cbStatus;                       // 状态(未支付(1), 已支付(2), 下单出错(3), 回调出错(4))
	CT_DWORD        dwTime;                         // 充值时间戳
	CT_DOUBLE       dPay;                           // 充值额
	CT_CHAR 		szOrderId[RECHARGE_OREDER_LEN];	// 订单号
};

//兑换结果
struct MSG_SC_ExchangeScore_Result : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			// 兑换结果(0: 领取成功, 1: 钻石不够, 2: 没有找相关的兑换等级)
	CT_WORD			wConsumeGem;		// 消耗钻石
	CT_LONGLONG		llAddScore;			// 获得金币
};

//积分相关的数据
struct MSG_SC_UserBaseScore : public MSG_GameMsgDownHead
{
	CT_DWORD        dwWinCount;											//赢得次数
	CT_DWORD        dwLostCount;										//输的次数
	CT_DWORD        dwDrawCount;										//和的次数
	CT_DWORD        dwFleeCount;										//逃跑次数
	CT_DWORD		dwTotalRecharge;									//充值总金额
	CT_BYTE			cbVipLevel;											//VIP等级
};

//绑定代理ID
struct MSG_SC_Bind_Promoter : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//绑定结果（0: 绑定成功 1:失败,已经绑定,不能重新绑定 2:没有找到这个代理ID）
	CT_WORD			wRewardGem;			//奖励钻石（奖励钻石）
	CT_DWORD		dwPromoterID;		//绑定的代理ID
};

//绑定帐号
struct MSG_SC_Bind_Mobile : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;					//绑定结果（0: 绑定成功 1:验证码错误 2:该手机已经绑定其他帐号）
	CT_CHAR			szNickName[NICKNAME_LEN];	//绑定后的昵称
};

//设置银行密码返回
struct MSG_SC_Set_BankPwd : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0: 设置成功 1: 密码不能为空 2:已经设置了密码, 不能重新设置)
};

//修改银行密码返回
struct MSG_SC_Modify_BankPwd : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0: 修改成功 1: 密码不能为空 2:原密码错误)
};

//银行存款
struct MSG_SC_Save_Score_To_Bank : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0: 操作成功  1: 存款金额超过身上金币 2: 在游戏中不能执行存款操作)
};

//银行取款
struct MSG_SC_Take_Score_From_Bank : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0: 操作成功 1: 密码错误 2: 取款金额超过银行金币 3: 在游戏中不能执行取款操作)
};

//更新银行金币
struct MSG_SC_Update_BankScore : public MSG_GameMsgDownHead
{
	CT_DOUBLE		dBankScore;			//当前银行金币
};

//获取验证码返回
struct MSG_SC_VerificationCode : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0:获取成功  1: 手机号码无效,请重新输入 2: 该手机已经绑定帐号(对于注册)  3:该手机未注册帐号(对于找回密码))
};

//重置密码
struct MSG_SC_Reset_Pwd : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 (0:修改成功 1: 该手机号未注册帐号 2: 验证码错误 3:密码不能为空)
};

//修改个人返回
struct MSG_SC_Modify_PersonalInfo : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果(0:修改成功 1: 性别不对 2:头像索引不对)
};

//绑定银行卡或者支付宝返回
struct MSG_SC_Bind_ExchangeAccount : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 （0: 绑定成功 1: 支付宝帐号不合法 2:银行卡号不合法 3:还没有绑定支付宝不能修改 4:验证码不正确）
};

//兑换返回
struct MSG_SC_Exchange_To_Rmb : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;			//结果 （0: 兑换成功请等待审核打款 1: 在游戏中不能进行兑换 2:还没有绑定支付宝 3: 还没有绑定银行卡 4:金币不足够 5:此支付宝帐号被禁止兑换 6:此银行卡号被禁止交易
	                                    // 7:今日支付宝达到最大兑换次数 8:今日银联达到最大兑换交次数 9:兑换金币没有达到最小兑换金额）
};

//兑换返回
struct MSG_SC_Exchange_Rmb_Record
{
	CT_CHAR			szAccount[ALIPAY_ACCOUNT_LEN];		//支付宝帐号或者银行帐号
	CT_DWORD		dwExchangeScore;					//兑换金币
	CT_BYTE			cbState;							//兑换状态(审核中(8), 已驳回(9), 准备兑换(10), 兑换失败(15), 兑换成功(18), 已退款(24))
	CT_CHAR			szTime[TIME_LEN];					//兑换时间
};

//已经绑定手机返回
struct MSG_SC_BindMobile_Already : public MSG_GameMsgDownHead
{
	CT_CHAR		szMobileNum[MOBILE_NUM_LEN];		//手机号
};

//绑定手机赠送金币
struct MSG_SC_BindMobile_PresentScore : public MSG_GameMsgDownHead
{
	CT_DOUBLE 	dBindMobileScore;
};

//举报返回
struct MSG_SC_Report_User_ForPlaza : public MSG_GameMsgDownHead
{
    CT_WORD     wGameID;                //游戏ID
    CT_WORD     wKindID;                //类型ID
    CT_DWORD    dwDrawID;               //记录ID
	CT_BYTE 	cbResult;				//举报结果(0：举报成功 1：已经举报过本局游戏 2：金币不足够 3: 在游戏中不能举报)
};

//机器连接返回
struct MSG_SC_AndroidConnect
{
	CT_BYTE		cbResult;		// 结果(0: 成功 1: 错误的机器标识)
};

//游戏版本号
struct MSG_SC_GameVersion
{
	CT_WORD		wGameID;		//游戏标识(由gamid*100+kindid生成)
	CT_WORD		wVersion;		//版本号
};

/////////////////////////////////////////////////////////////////////////////////////
//群组相关
#define		MSG_GROUP_MAIN								9

#define		SUB_CS_CREATE_GROUP							9001			//创建群组【MSG_CS_Create_Group】
#define     SUB_CS_MODIFY_GROUP_NAME                    9002            //修改群名称【MSG_CS_Modify_GroupName】
#define     SUB_CS_MODIFY_GROUP_NOTICE                  9003            //修改群公告【MSG_CS_Modify_GroupNotice】
#define     SUB_CS_MODIFY_GROUP_ICON                    9004            //修改群图标【MSG_CS_Modify_GroupIcon】
#define     SUB_CS_UPGRADE_GROUP                        9005            //升级群【MSG_CS_Upgrade_Group】
#define     SUB_CS_QUERY_GROUP_DIR_USER_LIST            9006            //查询直属成员列表【MSG_CS_Query_DirGroupUser_List】
#define     SUB_CS_QUERY_GROUP_DIR_USER_INFO            9007            //查询直属成员信息【MSG_CS_Query_DirGroupUser_Info】
#define     SUB_CS_QUERY_GROUP_SUB_USER_LIST            9008            //查询附属玩家成员列表信息【MSG_CS_Query_SubGroupUser_List】
#define     SUB_CS_MODIFY_GROUP_USER_REMARKS            9009            //修改个人备注【MSG_CS_DirGroupUser_Remarks】
#define     SUB_CS_UPGRADE_GROUP_USER_INCOMERATE        9010            //提升收入比例【MSG_CS_Upgrade_GroupUser_IncomeRate】
#define     SUB_CS_SEND_GROUP_CHAT_MSG                  9011            //发送群信息【MSG_CS_Send_GroupChatMsg】
#define     SUB_CS_QUERY_GROUP_INCOME                   9012            //查询收入【MSG_CS_Query_Group_Income】
#define     SUB_CS_QUERY_GROUP_YESTERDAY_INCOME         9013            //查询昨天收入明细【MSG_CS_Query_Group_Yesterday_Income】
#define     SUB_CS_QUERY_GROUP_SETTLE_RECORD            9014            //查询结算记录【MSG_CS_Query_Group_Settle_Record】
#define     SUB_CS_QUERY_GROUP_SETTLE_INFO              9015            //查询结算信息【MSG_CS_Query_Group_Settle_Info】
#define     SUB_CS_SETTLE_INCOME                        9016            //结算结利【MSG_CS_Settle_Income】
#define     SUB_CS_CREATE_GROUP_PROOM                   9017            //创建房间【MSG_CS_Create_Group_PRoom】
#define     SUB_CS_OPEN_PROOM_LIST_UI                   9018            //打开房间列表UI【MSG_CS_Open_Group_Game】
#define     SUB_CS_CLOSE_PROOM_LIST_UI                  9019            //关闭房间列表UI【MSG_CS_Close_Group_Game】
#define     SUB_CS_QUERY_GROUP_INCOME_DETAIL            9020            //查询明细记录【MSG_CS_Query_Group_Income_Detail】
#define     SUB_CS_QUERY_ONE_GROUP_DIR_USER             9021            //查询某个直属成员【MSG_CS_Query_One_Group_DirUser】
#define     SUB_CS_GIFT_SCORE_TO_DIR_USER               9022            //赠送金币给直属成员【MSG_CS_Gift_Score_DirUser】
#define     SUB_CS_READ_GROUP_MSG                       9023            //阅读群消息【MSG_CS_Read_Group_Msg】

//#define		SUB_CS_GROUP_CREATE_PROOM					9020			//创建群组私人房间【MSG_CS_Create_GroupPRoom】
//#define		SUB_CS_APPLY_ADD_GROUP						9006			//申请加入群【MSG_CS_Apply_AddGroup】
//#define		SUB_CS_OPERATE_APPLY_ADD_GROUP				9007			//处理加入群申请【MSG_CS_Operate_Apply_AddGroup】
//#define		SUB_CS_QUIT_GROUP							9008			//退出群【MSG_CS_Quit_Group】
//#define		SUB_CS_TICK_GROUP_USER						9009			//删除群成员【MSG_CS_Tick_GroupUser】
//#define		SUB_CS_DIMISS_GROUP							9011			//解散群【MSG_CS_Dismiss_Group】
//#define		SUB_CS_MODIFY_GROUP_INFO					9012			//修改群信息(群名称和群公告)【MSG_CS_Modify_GroupInfo】

//#define		SUB_CS_QUERY_GROUP_USER_INFO				9021			//查询用户信息【MSG_CS_Query_GroupUserInfo】
//#define     SUB_CS_CHANGE_GROUP_POSITION                9022            //改变成员职位【MSG_CS_Change_GroupPosition】
//#define		SUB_CS_QUERY_GROUP_INFO						9023			//查询群信息【MSG_CS_Query_GroupInfo弃用】
//#define		SUB_CS_PULL_USER_INTO_GROUP					9024			//群主拉人入群【MSG_CS_Pull_User_IntoGroup弃用】
//#define		SUB_CS_RECOMMEND_ADD_GROUP					9025			//推荐加入群【MSG_CS_Recommend_AddGroup弃用】

#define     SUB_SC_USER_GROUP_STATE                     9050            //玩家群组状态
#define		SUB_SC_GROUP_LIST							9051			//玩家群组列表【一组MSG_SC_GroupInfo】
#define     SUB_SC_GROUP_PARENT_USER                    9052            //玩家上级成员列表【一个MSG_SC_GroupUser】
#define		SUB_SC_GROUP_USER_LIST						9053			//玩家直属成员列表【一组MSG_SC_GroupUser】
#define     SUB_SC_GROUP_DIR_USER_LIST                  9054            //查询直属成员返回【1个MSG_SC_DirGroupUser_Summary+20条MSG_SC_DirGroupUser】
#define     SUB_SC_QUERY_GROUP_DIR_USER_INFO            9055            //查询直属玩家的信息返回【MSG_SC_DirGroupUser_Info】
#define     SUB_SC_QUERY_GROUP_SUB_USER_LIST            9056            //查询附属成员列表返回【一个MSG_SC_SubGroupUser_Summary+20条MSG_SC_SubGroupUser】
#define     SUB_SC_REC_GROUP_CHAT_MSG                   9057            //接收群聊天信息【MSG_SC_Send_GroupChatMsg】
#define     SUB_SC_MODIFY_GROUP_USER_REMARKS            9058            //修改直属成员备注返回【MSG_SC_DirGroupUser_Remarks】
#define     SUB_SC_UPGRADE_GROUP_USER_INCOMERATE        9059            //提升直属成员的收入比例【MSG_SC_Upgrade_GroupUser_IncomeRate】
#define     SUB_SC_QUERY_GROUP_INCOME                   9060            //查询收入返回【MSG_SC_Query_Group_Income】
#define     SUB_SC_QUERY_GROUP_YESTERDAY_INCOME         9061            //查询昨天收入明细【1条MSG_SC_Yesterday_Summary和20条MSG_SC_Query_Group_Yesterday_Income】
#define     SUB_SC_QUERY_GROUP_SETTLE_RECORD            9062            //查询结算记录【一组MSG_SC_Query_Group_Settle_Record】
#define     SUB_SC_SETTLE_INCOME                        9063            //结算结利【MSG_SC_Settle_Income】
#define     SUB_SC_GROUP_USER_ONLINE_STATE              9064            //玩家在线状态【MSG_SC_Group_Online_State】
#define     SUB_SC_GROUP_ADD_DIR_USER                   9065            //增加直属成员【MSG_SC_Add_GroupUser】
#define		SUB_SC_CREATE_GROUP							9066			//创建群组返回【MSG_SC_CreateGroup】
#define		SUB_SC_MODIFY_GROUP_INFO					9067			//修改群信息返回【MSG_SC_Modify_GroupInfo】修改名称 公告 ICON 升级公会
#define     SUB_SC_QUERY_GROUP_SETTLE_INFO              9068            //查询结算信息【MSG_SC_Query_Group_Settle_Info】
#define     SUB_SC_CREATE_GROUP_PROOM                   9069            //创建房间返回【MSG_SC_Create_Group_PRoom】
#define     SUB_SC_PROOM_LIST                           9070            //房间列表返回【一个MSG_SC_Group_PRoomID和一组MSG_SC_Group_PRoom_List】
#define     SUB_SC_PROOM_UPDATE                         9071            //房间信息更新【一个MSG_SC_Group_PRoomID和一个MSG_SC_Group_PRoom_List】
#define     SUB_SC_PROOM_ADD                            9072            //房间增加【一个MSG_SC_Group_PRoomID和一个MSG_SC_Group_PRoom_List】
#define     SUB_SC_PROOM_REMOVE                         9073            //房间信息删除【MSG_SC_Group_PRoom_Remove】
#define     SUB_SC_QUERY_GROUP_INCOME_DETAIL            9074            //查询汇总收入明细【1条MSG_SC_Yesterday_Summary和20条MSG_SC_Query_Group_Yesterday_Income】
#define     SUB_SC_QUERY_ONE_GROUP_DIR_USER             9075            //查询某个玩家返回【MSG_SC_One_DirGroupUser】
#define     SUB_SC_GIFT_SCORE_TO_DIR_USER               9076            //赠送金币给直属成员【MSG_SC_Gift_Score_DirUser】
#define     SUB_SC_UPDATE_GROUP_CONTRIBUTION            9077            //更新群贡献度【MSG_SC_Update_Group_Contribution】
#define     SUB_SC_UPDATE_GROUP_USER_COUNT              9078            //更新群人数【MSG_SC_Update_GroupUser_Count】
#define     SUB_SC_UPDATE_GROUP_PROMOTION_DOMAIN        9079            //更新群推广连接【MSG_SC_Update_GroupPromotion_Domain】
#define     SUB_SC_UPGRADE_GROUP                        9080            //升级公会返回【MSG_SC_Upgrade_Group】

#define     SUB_SC_SHOW_GROUP_BTN                       9090            //是否显示公会按钮【MSG_SC_Show_GroupBtn】

//以下暂时弃用
#define		SUB_SC_GROUP_APPLY_LIST						9065			//群组申请列表【一组MSG_SC_ApplyGroup】
//#define		SUB_SC_CREATE_GROUP							9070			//创建群组返回【MSG_SC_CreateGroup】
#define		SUB_SC_APPLY_ADD_GROUP						9071			//申请加入群组返回（只有群主能收到）【MSG_SC_ApplyGroup】
#define		SUB_SC_ADD_GROUP_USER						9072			//对于已经在群加入群组返回【MSG_SC_Add_GroupUser】
#define		SUB_SC_ADD_GROUP_INFO						9073			//新加群组返回【MSG_SC_GroupInfo】
#define		SUB_SC_ADD_GROUP_USER_LIST					9074			//新加群组成员返回【一组MSG_SC_GroupUser】			
#define		SUB_SC_QUIT_GROUP_USER						9075			//退出群组返回【MSG_SC_Remove_GroupUser】
#define		SUB_SC_TICK_GROUP_USER						9076			//被踢出群组返回【MSG_SC_Remove_GroupUser】
#define		SUB_SC_GROUP_CREATE_PROOM					9077			//群创建房间返回【MSG_SC_Create_GroupPRoom】
#define		SUB_SC_DISMISS_GROUP						9078			//解散群返回【MSG_SC_Dismiss_Group】
#define		SUB_SC_QUERY_GROUP_INFO						9079			//查询群信息返回【MSG_SC_Query_GroupInfo】
#define		SUB_SC_QUERY_GROUP_USER_INFO			    9081			//查询用户信息返回【一组MSG_SC_Query_GroupUserInfo】
#define     SUB_SC_CHANGE_GROUP_POSITION                9082            //修改成员职位【MSG_SC_Change_GroupPosition】
#define     SUB_SC_OP_APPLY_ADD_GROUP                   9083            //处理申请列表返回【MSG_SC_Op_Apply_AddGroup】

#define		SUB_SC_GROUP_PROOM_CLEARING_INFO			9090			//大结算信息【一个MSG_SC_GroupPRoom_Clearing】

#define		SUB_SC_GROUP_ERROR							9099			//群组错误返回【MSG_SC_Group_Error】

//一定要看
//**********************************************************
//***
//群组所有上行消息的结构第一个变量一定是userid。
//***
//**********************************************************

//创建群组
struct MSG_CS_Create_Group
{
	CT_DWORD	dwUserID;								//用户ID
	CT_BYTE		cbIcon;									//图标
	CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
	CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

//申请加入群组
struct MSG_CS_Apply_AddGroup
{
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwGroupID;								//群组ID
};

//处理申请加入群组
struct MSG_CS_Operate_Apply_AddGroup
{
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwApplyUserID;							//申请者ID
	CT_BYTE		cbOperateCode;							//操作码(0表示不同意, 1表示同意)
};

//退出群
struct MSG_CS_Quit_Group
{
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwGroupID;								//群组ID
};

//删除群成员
struct MSG_CS_Tick_GroupUser
{
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwGroupID;								//群组ID
};

//创建私人房间
struct MSG_CS_Create_GroupPRoom
{
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwGroupID;								//群组ID
	CT_WORD		wGameID;								//游戏ID
	CT_WORD		wKindID;								//类型ID
};

//删除群信息
struct MSG_CS_Dismiss_Group
{
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwGroupID;								//群组ID
};

//修改群信息
struct MSG_CS_Modify_GroupInfo
{
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwGroupID;								//群组ID
	CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
	CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

//修改群名称
struct MSG_CS_Modify_GroupName
{
    CT_DWORD    dwMasterID;                             //群主ID
    CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
};

//修改群公告
struct MSG_CS_Modify_GroupNotice
{
    CT_DWORD    dwMasterID;                             //群主ID
    CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

//修改群图标
struct MSG_CS_Modify_GroupIcon
{
    CT_DWORD    dwMasterID;                             //群主ID
    CT_BYTE     cbIcon;                                 //群图标
};

//升级群
struct MSG_CS_Upgrade_Group
{
    CT_DWORD    dwMasterID;                             //群主ID
};

//修改备注
struct MSG_CS_DirGroupUser_Remarks
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwDirUserID;                            //直属成员ID
    CT_CHAR     szRemarks[GROUP_REMARKS_LEN];           //备注
};

//提升群成员收入比例
struct MSG_CS_Upgrade_GroupUser_IncomeRate
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwDirUserID;                            //直属成员ID
    CT_BYTE     cbNewIncomeRate;                        //直属成员新收入比例
};

//查询直属成员列表
struct MSG_CS_Query_DirGroupUser_List
{
    CT_DWORD    dwUserID;                              //成员ID
    CT_WORD     wPage;                                 //第几页
};

//查询直属成员的信息
struct MSG_CS_Query_DirGroupUser_Info
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwDirUserID;                            //直属成员ID
};

//查询附属成员列表
struct MSG_CS_Query_SubGroupUser_List
{
    CT_DWORD    dwUserID;                              //成员ID
    CT_DWORD    dwDirUserID;                           //直属成员ID
    CT_WORD     wPage;                                 //第几页
};

//查询群信息
struct MSG_CS_Query_GroupInfo
{
	CT_DWORD	dwUserID;								//玩家ID
	CT_DWORD	dwGroupID;								//群ID
};

//群聊天
struct MSG_CS_Send_GroupChatMsg
{
    CT_DWORD    dwFromUserID;                           //发送者
    CT_DWORD    dwToUserID;                             //接收者
    CT_CHAR	    szContent[300];					        //聊天内容
};

//查询收入
struct MSG_CS_Query_Group_Income
{
    CT_DWORD    dwUserID;                               //用户ID
};

//查询昨天收入明细
struct MSG_CS_Query_Group_Yesterday_Income
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_WORD     wPage;                                  //第几页
};

//查询汇总明细
struct MSG_CS_Query_Group_Income_Detail
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_WORD     wPage;                                  //第几页
};

//查询某个直属成员
struct MSG_CS_Query_One_Group_DirUser
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwBeQueryUserID;                        //被查询用户
};

//赠送金币
struct MSG_CS_Gift_Score_DirUser
{
    CT_DWORD    dwUserID;                               //玩家ID
    CT_DWORD    dwDirUserID;                            //被赠送玩家ID
    CT_DWORD    dwGiftScore;                            //赠送金币
};

//阅读群消息
struct MSG_CS_Read_Group_Msg
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwMsgID;                                //消息ID
};

//查询结算记录
struct MSG_CS_Query_Group_Settle_Record
{
    CT_DWORD    dwUserID;                               //用户ID
};

//查询结算红利信息
struct MSG_CS_Query_Group_Settle_Info
{
    CT_DWORD    dwUserID;                               //查询用户结算
};

//结算结利
struct MSG_CS_Settle_Income
{
    CT_DWORD    dwUserID;                               //用户ID
};

//创建群组私人房间
struct MSG_CS_Create_Group_PRoom
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_WORD     wGameID;                                //游戏ID
    CT_WORD     wKindID;                                //游戏类型ID
    CT_WORD     wRoomKindID;                            //房间类型
    CT_BYTE     cbLock;                                 //是否加密房间(0不加密码 1加密)
};

//打开哪个群组游戏界面
struct MSG_CS_Open_Group_Game
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_WORD     wGameID;                                //游戏ID
    CT_WORD     wKindID;                                //游戏类型ID
};

//关闭群组游戏
struct MSG_CS_Close_Group_Game
{
    CT_DWORD    dwUserID;                               //用户ID
};

//拉人进群
struct MSG_CS_Pull_User_IntoGroup
{
	CT_DWORD	dwMasterID;								//群主ID	
	CT_DWORD	dwPulledUserID;							//被拉用户
	CT_DWORD	dwGroupID;								//群组ID
};

//推荐加入群
struct MSG_CS_Recommend_AddGroup
{
	CT_DWORD	dwUserID;								//推荐玩家ID
	CT_DWORD	dwRecommendedUserID;					//被推荐玩家ID
	CT_DWORD	dwGroupID;								//群组ID
};

//改变群成员职位
struct MSG_CS_Change_GroupPosition
{
    CT_DWORD	dwMasterID;								//群主ID
    CT_DWORD    dwUserID;                               //被操作的玩家ID
    CT_DWORD    dwGroupID;                              //群组ID
    CT_BYTE 	cbOperateCode;							//操作代码(1: 设为副群主 2: 设为普通成员)
};

//查询用户信息
struct MSG_CS_Query_GroupUserInfo
{
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwGroupID;						        //群组ID
};

//群组信息
struct MSG_SC_GroupInfo
{
    MSG_SC_GroupInfo()
    {
        memset(this, 0, sizeof(*this));
    }
	CT_DWORD		dwGroupID;							//群组ID
	CT_DWORD		dwMasterID;							//群主ID
	CT_DWORD        dwContribution;                     //当前贡献值
	CT_DWORD 		dwNextContribution;					//下一个等级的贡献值
	CT_DWORD        dwGroupUserCount;                   //公会总人数
	CT_BYTE         cbLevel;                            //等级
	CT_BYTE         cbIcon;                             //图标
    CT_BYTE         cbIncomeRate;                       //公会红利比例
    CT_BYTE         cbSettleDays;                       //结算天数
	CT_CHAR			szGroupName[GROUP_NAME_LEN];		//群名称
	CT_CHAR			szNotice[GROUP_NOTICE_LEN];			//群公告
	CT_CHAR         szDomain[GROUP_PROMOTION_DOMIAN];   //推广主页
};

//群组成员信息(用于聊天的成员信息)
struct MSG_SC_GroupUser
{
    MSG_SC_GroupUser()
    {
        memset(this, 0, sizeof(*this));
    }
	CT_DWORD		dwUserID;							//用户ID
	CT_BYTE         cbSex;                              //性别
	CT_BYTE         cbHeadId;                           //头像
	CT_BYTE         cbVip2;                             //VIP2
    CT_BYTE         cbOnline;                           //是否在线
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //玩家备注
    //CT_BYTE         cbIncomeRate;                       //收入比例
	//CT_WORD         wSubUserCount;                      //附属军团人数
	//CT_DWORD       nddwLastLoginDate;					//最后登录时间
};

//群直属成员汇总信息
struct MSG_SC_DirGroupUser_Summary
{
    MSG_SC_DirGroupUser_Summary()
    :wDirSubUserCount(0)
    ,wDirUserCount(0)
    ,wCurrPage(0)
    ,wTotalPage(0)
    {

    }
    CT_WORD         wDirSubUserCount;                   //组织成员数量
    CT_WORD         wDirUserCount;                      //直属成员数量
    CT_WORD         wCurrPage;                          //当前页数
    CT_WORD         wTotalPage;                         //总页数
};

//群直属成员
struct MSG_SC_DirGroupUser
{
    MSG_SC_DirGroupUser()
    :dwUserID(0)
    ,dwLastLoginTime(0)
    ,wSubUserCount(0)
    ,cbIncomeRate(0)
    ,cbOnline(0)
    {
        szRemarks[0] = '\0';
    }
    CT_DWORD        dwUserID;                           //用户ID
    CT_DWORD        dwLastLoginTime;                    //最后登录时间
    CT_WORD         wSubUserCount;                      //此用户附属军团的人数(包括直属成员)
    CT_BYTE         cbIncomeRate;                       //红利比例
    CT_BYTE         cbOnline;                           //是否在线(0 不在线 1 在线)
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //玩家备注
};

//查询直属成员返回
struct MSG_SC_One_DirGroupUser : public MSG_GameMsgDownHead
{
    MSG_SC_One_DirGroupUser()
    :cbResult(1)
    {
    }

    CT_BYTE             cbResult;                       //结果(0 成功 1没有找这个直属成员)
    MSG_SC_DirGroupUser dirUserInfo;                    //返回的结果
};

//赠送金币返回
struct MSG_SC_Gift_Score_DirUser : public MSG_GameMsgDownHead
{
    MSG_SC_Gift_Score_DirUser()
    :cbResult(0)
    {
    }

    CT_BYTE             cbResult;                       //结果(0 成功 1此玩家非直属成员 2金币不足够 3游戏中不能赠送金币)
};

//更新群贡献度
struct MSG_SC_Update_Group_Contribution : public MSG_GameMsgDownHead
{
    CT_DWORD            dwContribution;                 //当前贡献度
};

//更新群人数
struct MSG_SC_Update_GroupUser_Count : public MSG_GameMsgDownHead
{
    CT_WORD             wUserCount;                     //用户ID
};

//更新群推广链接
struct MSG_SC_Update_GroupPromotion_Domain : public MSG_GameMsgDownHead
{
    CT_CHAR             szDomain[GROUP_PROMOTION_DOMIAN];
};

//群直属成员的详细信息
struct MSG_SC_DirGroupUser_Info : public MSG_GameMsgDownHead
{
    CT_DWORD        dwUserID;                           //成员ID
    CT_DWORD        dwRecharge;                         //成员总充值
    CT_DOUBLE       dIncome;                            //成员红利
    CT_WORD         wSubUserCount;                      //附属成员军团人数
    CT_BYTE         cbSelfIncomRate;                    //上级的红利比例
    CT_BYTE         cbDirIncomeRate;                    //红利比例
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //备注
};

//附属成员汇总信息
struct MSG_SC_SubGroupUser_Summary
{
    MSG_SC_SubGroupUser_Summary()
    :wCurrPage(0)
    ,wTotalPage(0) {}
    CT_WORD         wCurrPage;                          //当前页数
    CT_WORD         wTotalPage;                         //总页数
};

//附属属成员
struct MSG_SC_SubGroupUser
{
    MSG_SC_SubGroupUser()
        :dwUserID(0)
        ,dwRegTime(0)
        ,dwLastLoginTime(0)
        ,dwRecharge(0)
        ,dIncome(0.0f)
    { }
    CT_DWORD        dwUserID;                           //用户ID
    CT_DWORD        dwRegTime;                          //注册时间
    CT_DWORD        dwLastLoginTime;                    //最后登录时间
    CT_DWORD        dwRecharge;                         //充值金额
    CT_DOUBLE       dIncome;                            //红利
};

//修改直属成员备注
struct MSG_SC_DirGroupUser_Remarks : public MSG_GameMsgDownHead
{
    CT_DWORD        dwDirUserID;                        //直属成员ID
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //新备注
};

//修改直属成员的收入比例
struct MSG_SC_Upgrade_GroupUser_IncomeRate : public MSG_GameMsgDownHead
{
    CT_DWORD        dwDirUserID;                        //直属成员ID
    CT_BYTE         cbIncomeRate;                       //成员比例
};

//显示公会按钮
struct MSG_SC_Show_GroupBtn : public MSG_GameMsgDownHead
{
    CT_BYTE         cbShow;                             //0不显示 1显示
};

//聊天消息接收
struct MSG_SC_Send_GroupChatMsg : public MSG_GameMsgDownHead
{
    CT_DWORD        dwMsgID;                            //消息ID
    CT_DWORD        dwFromUserID;                       //消息发送者
    CT_DWORD        dwToUserID;                         //消息接收者
    CT_CHAR	        szContent[300];					    //聊天内容
    CT_DWORD        dwSendTime;                         //发送时间
};

//查询收入返回
struct MSG_SC_Query_Group_Income : public MSG_GameMsgDownHead
{
    MSG_SC_Query_Group_Income()
    :dTodayDirIncome(0.0f)
    ,dTodaySubIncome(0.0f)
    ,cbSelfIncomeRate(0)
    ,cbSettleDays(0)
    ,wSubGroupUserCount(0)
    ,dYesterDayIncome(0.0f)
    ,dTotalIncome(0.0f)
    ,dRemainingIncome(0.0f)
    { }

    CT_DOUBLE       dTodayDirIncome;                    //今天直属成员红利
    CT_DOUBLE       dTodaySubIncome;                    //今天附属成员红利
    CT_BYTE         cbSelfIncomeRate;                   //我的红利比例
    CT_BYTE         cbSettleDays;                       //结算天数
    CT_WORD         wSubGroupUserCount;                 //我的军团总人数
    CT_DOUBLE       dYesterDayIncome;                   //昨日军团红利
    CT_DOUBLE       dTotalIncome;                       //军团总红利
    CT_DOUBLE       dRemainingIncome;                   //剩余结算红利
};

//查询昨天收入明细汇总
struct MSG_SC_Yesterday_Summary
{
    MSG_SC_Yesterday_Summary()
    :wCurrPage(0)
    ,wTotalPage(0)
    ,wDirSubGroupUserCount(0)
    ,wDirGroupUserCount(0)
    ,cbIncomeRate(0)
    ,dDirIncome(0.0f)
    ,dSubIncome(0.0f)
    {
    }
    CT_WORD         wCurrPage;                          //当前页数
    CT_WORD         wTotalPage;                         //总页数
    CT_WORD         wDirSubGroupUserCount;              //军团总人数
    CT_WORD         wDirGroupUserCount;                 //直属人数
    CT_BYTE         cbIncomeRate;                       //我的红利比例
    CT_DOUBLE       dDirIncome;                         //直属红利
    CT_DOUBLE       dSubIncome;                         //附属红利
};

//查询昨天收入明细返回
struct MSG_SC_Query_Group_Yesterday_Income
{
    MSG_SC_Query_Group_Yesterday_Income()
    :dwUserID(0)
    ,cbIncomeRate(0)
    ,dDirIncome(0.0f)
    ,wSubGroupUserCount(0)
    ,dSubIncome(0.0f)
    {
        szRemarks[0] = '\0';
    }
    CT_DWORD        dwUserID;                           //玩家ID
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //玩家昵称
    CT_BYTE         cbIncomeRate;                       //红利比例
    CT_DOUBLE       dDirIncome;                         //直属红利
    CT_WORD         wSubGroupUserCount;                 //附属军团人数
    CT_DOUBLE       dSubIncome;                         //附属红利
};

//查询结算记录返回
struct MSG_SC_Query_Group_Settle_Record
{
    CT_DWORD        dwSubmitTime;                       //提交时间
    CT_CHAR         szOrderNum[GROUP_ORDER_LEN];        //订单号
    CT_DOUBLE       dAmount;                            //金额
    CT_BYTE         cbStatus;                           //状态
    CT_DWORD        dwHandleTime;                       //处理时间
};

//查询可结算红利
struct MSG_SC_Query_Group_Settle_Info : public MSG_GameMsgDownHead
{
    MSG_SC_Query_Group_Settle_Info()
    :cbSettleDays(0)
    ,dRemainingIncome(0.0f)
    ,dCanSettleIncome(0.0f)
    //,dLastSettleIncome(0.0f)
    { }

    CT_BYTE         cbSettleDays;                       //结算天数
    CT_DOUBLE       dRemainingIncome;                   //剩余结算红利
    CT_DOUBLE       dCanSettleIncome;                   //可结算红利
    //CT_DOUBLE       dLastSettleIncome;                  //上次结算红利
};

//结算结利返回
struct MSG_SC_Settle_Income : public MSG_GameMsgDownHead
{
    MSG_SC_Settle_Income()
    :cbResult(0)
    ,dRemainingIncome(0.0f)
    ,dCanSettleIncome(0.0f)
    ,dSettleMinIncome(0.0f){}

    CT_BYTE         cbResult;                           //结果(0成功 1没有可结算红利 2结算未达到最小结算标准)
    CT_DOUBLE       dRemainingIncome;                   //剩余结算红利
    CT_DOUBLE       dCanSettleIncome;                   //可结算红利
    CT_DOUBLE       dSettleMinIncome;                   //最小结算红利
};

//创建房间返回
struct MSG_SC_Create_Group_PRoom : public MSG_GameMsgDownHead
{
    MSG_SC_Create_Group_PRoom()
    :cbResult(0)
    ,wGameID(0)
    ,wKindID(0)
    ,dwRoomNum(0)
    ,dwServerID(0) { }

    CT_BYTE         cbResult;                           //结果(0成功 1该游戏私人房间过多稍后再试 2未找到空闲的房间服务器 3玩家的金币不够创建此房间)
    CT_WORD         wGameID;                            //游戏ID
    CT_WORD         wKindID;                            //类型ID
    CT_DWORD        dwRoomNum;                          //房间ID
    CT_DWORD        dwServerID;                         //房间所在的服务器ID
};

//房间类型
struct MSG_SC_Group_PRoomID
{
    CT_WORD         wGameID;                            //游戏ID
    CT_WORD         wKindID;                            //类型ID
};

//房间信息返回
struct MSG_SC_Group_PRoom_List
{
    CT_WORD         wRoomKindID;                        //房间类型
    CT_DWORD        dwRoomNum;                          //房间号
    CT_DWORD        dwOwnerID;                          //房间所有者
    CT_DWORD        dwServerID;                         //服务器ID
    CT_BYTE         cbLock;                             //是否锁定
    CT_DWORD        dwUserID[MAX_PLAYER];               //桌子玩家
    CT_BYTE         cbHeadID[MAX_PLAYER];               //头像ID
    CT_BYTE         cbSex[MAX_PLAYER];                  //性别
    CT_BYTE         cbVip2[MAX_PLAYER];                 //Vip2等级
};

//房间删除
struct MSG_SC_Group_PRoom_Remove : public MSG_GameMsgDownHead
{
    CT_WORD         wGameID;                            //游戏ID
    CT_WORD         wKindID;                            //类型ID
    CT_DWORD        dwRoomNum;                          //房间号
};

//直属玩家在线状态
struct MSG_SC_Group_Online_State : public MSG_GameMsgDownHead
{
    CT_DWORD        dwUserID;                           //直属成员ID
    CT_BYTE         cbOnline;                           //在线状态(0不在线 1在线)
};

//群组的房间信息
struct MSG_SC_Group_PRoom
{
	CT_DWORD		dwGroupID;							//群ID
	CT_DWORD		dwUserID;							//成员ID
	CT_CHAR			szJsonResult[PROOM_PARAM_LEN];		//开房Json结果
};

//申请加入群信息
struct MSG_SC_ApplyGroup
{
	CT_DWORD		dwGroupID;							//群信息
	CT_DWORD		dwUserID;							//用户ID
	CT_DWORD 		dwApplyTime;						//申请时间
    CT_BYTE         cbSex;                              //性别
    CT_BYTE         cbHeadId;                           //头像
    CT_BYTE         cbVip2;                             //VIP2
	//CT_CHAR			szNickName[NICKNAME_LEN];			//用户昵称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];			//用户头像
	//CT_CHAR			szApplyTime[TIME_LEN];				//申请时间
};

//结算信息
struct MSG_SC_GroupPRoom_Clearing
{
	CT_DWORD		dwGroupID;								//群组ID
	CT_DWORD		dwRoomNum;								//房间号
	CT_WORD			wGameID;								//游戏ID
	CT_WORD			wKindID;								//类型ID
	CT_BYTE			cbCount;								//数量
	CT_CHAR			szNickName[MAX_PLAYER][NICKNAME_LEN];	//玩家昵称
	CT_INT32		iScore[MAX_PLAYER];						//玩家分数
	CT_CHAR			szTime[TIME_LEN];						//申请时间
};

//群组状态
struct MSG_SC_UserGroup_State : public MSG_GameMsgDownHead
{
    CT_BYTE         cbState;                                //状态(1:可以创建 2:不能创建)
};

//创建群
struct MSG_SC_CreateGroup : public MSG_GameMsgDownHead
{
	MSG_SC_GroupInfo	groupInfo;						//群信息
	//MSG_SC_GroupUser	groupUser;						//群主信息
};

//增加群组成员
struct MSG_SC_Add_GroupUser : public MSG_GameMsgDownHead
{
    MSG_SC_Add_GroupUser()
    :dwUserID(0)
    ,cbSex(0)
    ,cbHeadId(0)
    ,cbVip2(0)
    ,cbOnline(0)
    {
        szRemarks[0] = '\0';
    }
    CT_DWORD		dwUserID;							//用户ID
    CT_BYTE         cbSex;                              //性别
    CT_BYTE         cbHeadId;                           //头像
    CT_BYTE         cbVip2;                             //VIP2
    CT_BYTE         cbOnline;                           //是否在线
    CT_CHAR         szRemarks[GROUP_REMARKS_LEN];       //玩家昵称
};

//处理申请加入群组返回
struct MSG_SC_Op_Apply_AddGroup
{
    CT_DWORD        dwGroupID;                          //群ID
    CT_DWORD        dwUserID;                           //申请用户ID
};

//删除群成员
struct MSG_SC_Remove_GroupUser : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGroupID;							//群ID
	CT_DWORD		dwUserID;							//用户ID
};

//创建房间返回
struct MSG_SC_Create_GroupPRoom : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGroupID;							//群ID
	CT_DWORD		dwUserID;							//用户ID
	CT_CHAR			szJsonResult[PROOM_PARAM_LEN];		//开房Json结果
};

//解散群返回
struct MSG_SC_Dismiss_Group : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGroupID;							//群ID
};

//查询群信息返回
struct MSG_SC_Query_GroupInfo : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGroupID;							//群组ID
	CT_DWORD		dwMasterID;							//群主ID
	CT_WORD			wUserCount;							//成员数量
	CT_CHAR			szMasterName[NICKNAME_LEN];			//群主昵称
	CT_CHAR			szGroupName[GROUP_NAME_LEN];		//群名称
};

//修改群信息返回
struct MSG_SC_Modify_GroupInfo : public MSG_GameMsgDownHead
{
    CT_BYTE     cbLevel;                                //群等级
    CT_BYTE     cbIcon;                                 //群图标
    CT_BYTE     cbSettleDays;                           //结算天数
    //CT_BYTE     cbIncomeRate;                           //收入比例
    CT_DWORD    dwNextContribution;                     //下一个等级的最大贡献度
    CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
    CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

//升级群返回
struct MSG_SC_Upgrade_Group : public MSG_GameMsgDownHead
{
    CT_BYTE     cbLevel;                                //等级
    CT_BYTE     cbSettleDays;                           //结算天数
    CT_BYTE     cbIncomeRate;                           //收入比例
    CT_DWORD    dwNextContribution;                     //下一个等级的最大贡献度
};

//用户信息返回
struct MSG_SC_Query_GroupUserInfo
{
	CT_WORD         wGameID;							//游戏ID
	CT_WORD         wKindID;							//类型ID
	CT_WORD 		wRoomKindID;						//房间类型
	CT_DWORD 		dwRoomCount;                        //开房数量
	CT_DOUBLE       dIncome;                            //开房收益
};

//修改成员职位返回
struct MSG_SC_Change_GroupPosition : public MSG_GameMsgDownHead
{
    CT_DWORD        dwGroupID;                          //群组ID
    CT_DWORD        dwUserID;                           //成员ID
    CT_BYTE         cbPosition;                         //成员职位(2:副群主 3:成员)
};

//修改群组默认游戏
struct MSG_SC_Modify_GroupDefaultGame : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGroupID;							//群组ID
	CT_DWORD		dwDefaultGameKind;					//默认游戏ID（gameid*100+kindid）
};

//群组错误返回
struct MSG_SC_Group_Error : public MSG_GameMsgDownHead
{
	CT_BYTE			cbErrorCode;						//错误码
};

/////////////////////////////////////////////////////////////////////////////////////
//客户端的查询命令
#define		MSG_QUERY_MAIN								20
#define		SUB_CS_QUERY_PROOM_GAMEROUND_INFO			2001			//查询大局信息【MSG_CS_Query_PRoom_GameRound】
#define		SUB_CS_QUERY_PROOM_PLAYCOUNT_INFO			2002			//查询每小局信息【MSG_CS_Query_PRoom_PlayCount】
#define		SUB_CS_QUERY_USER_SCORE_INFO				2003			//查询玩家金币类基本信息【MSG_CS_Query_UserScore_Info】
#define		SUB_CS_QUERY_USER_HONGBAO_INFO				2004			//查询玩家红包信息【MSG_CS_Query_UserHongBao_Info】
#define 	SUB_CS_QUERY_GAME_RECORD					2005			//查询玩家游戏记录【MSG_CS_Query_GameRecord】
#define 	SUB_CS_QUERY_REPORT_GAME_RECORD				2006			//查询举报的游戏记录[MSG_CS_Query_Report_GameRecord]
#define 	SUB_CS_QUERY_NORMAL_GAME_RECORD				2007			//查询正常的游戏记录[MSG_CS_Query_Normal_GameRecord]
#define 	SUB_CS_QUERY_HBSL_RECORD					2008			//查询红包扫雷抢红包记录【MSG_CS_Query_Hbsl_Record】
#define 	SUB_CS_QUERY_HBSL_FA_RECORD					2009			//查询红包扫雷发红包记录【MSG_CS_Query_Hbsl_Record】
#define 	SUB_CS_QUERY_HBSL_WIN_COLOR_PRIZE_RECORD	2010			//查询红包扫雷中彩奖记录【MSG_CS_Query_Hbsl_Record】

#define		SUB_SC_QUERY_PROOM_GAMEROUND_INFO			2050			//查询大局信息返回【MSG_SC_Query_PRoom_Gameround】
#define		SUB_SC_QUERY_PROOM_GAMEROUND_FINISH			2051			//查询每小局信息完成【无】
#define		SUB_SC_QUERY_PROOM_PLAYCOUNT_INFO			2052			//查询每小局信息返回【MSG_SC_Query_PRoom_PlayCount】
#define		SUB_SC_QUERY_PROOM_PLAYCOUNT_FINISH			2053			//查询每小局信息完成【无】
#define		SUB_SC_QUERY_USER_SCORE_INFO				2054			//查询玩家金币类基本信息【MSG_SC_Query_UserScore_Info】
#define		SUB_SC_QUERY_USER_HONGBAO_RECORD_INFO		2055			//查询玩家红包记录信息【MSG_SC_Query_UserHongBaoRecord_Info】
#define 	SUB_SC_QUERY_GAME_RECORD					2056			//查询玩家游戏记录返回[MSG_SC_Query_GameRecord_Head+一组MSG_SC_Query_GameRecord]
#define     SUB_SC_QUERY_HBSL_RECORD_START              2057            //查询红包扫雷抢红包记录开始
#define     SUB_SC_QUERY_HBSL_RECORD                    2058            //查询红包扫雷抢红包记录【MSG_SC_Query_HBSLRecord+一组MSG_SC_Query_HBSLRecord_Detail】
#define     SUB_SC_QUERY_HBSL_RECORD_FINISH             2059            //查询红包扫雷抢红包记录结束
#define     SUB_SC_QUERY_HBSL_FA_RECORD_START           2060            //查询包扫雷发红包记录开始
#define     SUB_SC_QUERY_HBSL_FA_RECORD                 2061            //查询包扫雷发红包记录【MSG_SC_Query_HBSLRecord+一组MSG_SC_Query_HBSLRecord_Detail】
#define     SUB_SC_QUERY_HBSL_FA_RECORD_FINISH          2062            //查询包扫雷发红包记录结束
#define     SUB_SC_QUERY_HBSL_COLOR_PRIZE_RECORD        2063            //查询抢发包扫雷记录【多组MSG_SC_Query_HBSLWinColorPrizeRecord】


//玩家查询大局信息(返回10)
struct MSG_CS_Query_PRoom_GameRound
{
	CT_DWORD		dwUserID;						// 玩家ID
	CT_WORD			wGameID;						// 游戏ID
	CT_WORD			wKindID;						// 游戏子ID
};

//查询每个大局中每小局的信息
struct MSG_CS_Query_PRoom_PlayCount
{
	CT_UINT64		uGameRoundID;					// 大局ID
	CT_DWORD		dwUserID;						// 查询的玩家ID
};

//查询玩家基本信息
struct MSG_CS_Query_UserScore_Info
{
	CT_DWORD		dwUserID;						//玩家ID
	CT_DWORD		dwQueryUserID;					//被查询的用户
};

//查询红包信息
struct MSG_CS_Query_UserHongBao_Info 
{
	CT_DWORD		dwUserID;						//玩家ID
};

//查询金币场游戏记录
struct MSG_CS_Query_GameRecord
{
	CT_DWORD		dwUserID;						// 玩家ID
	CT_WORD			wGameID;						// 游戏ID
	CT_WORD			wKindID;						// 游戏子ID
};

//查询举报的游戏记录
struct MSG_CS_Query_Report_GameRecord
{
	CT_DWORD		dwUserID;						//玩家ID
	CT_DWORD 		dwReportID;						//举报ID
};

//查询正常的游戏记录
struct MSG_CS_Query_Normal_GameRecord
{
	CT_DWORD		dwUserID;						//玩家ID
    CT_DWORD        dwDrawID;						//记录ID
    CT_CHAR 		szDate[DATE_LEN];				//日期
};

//查询玩家红包扫雷记录
struct MSG_CS_Query_Hbsl_Record
{
    CT_DWORD        dwUserID;                       //玩家ID
};

//玩家查询大局信息返回
struct MSG_SC_Query_PRoom_Gameround : public MSG_GameMsgDownHead
{
	CT_UINT64		uGameRoundID;
	CT_DWORD		dwRoomNum;
	CT_BYTE			cbUserCount;
	CT_DWORD		arrUserID[10];
	CT_CHAR			arrNickname[10][NICKNAME_LEN];
	CT_INT32		arrScore[10];
	CT_CHAR			szEndDate[DATE_LEN];
	CT_CHAR			szEndTime[TIME_LEN_2];
};

//玩家查询大局信息返回
struct MSG_SC_Query_PRoom_PlayCount : public MSG_GameMsgDownHead
{
	CT_UINT64		uGameRoundID;
	CT_BYTE			cbPlayCountID;
	CT_DWORD		dwRoomNum;
	CT_WORD			wSelfChairID;
	CT_BYTE			cbUserCount;
	CT_DWORD		arrUserID[10];
	CT_CHAR			arrNickname[10][NICKNAME_LEN];
	CT_INT32		arrScore[10];
	CT_CHAR			szEndDate[DATE_LEN];
	CT_CHAR			szEndTime[TIME_LEN_2];
};

//查询金币类基本信息
struct MSG_SC_Query_UserScore_Info : public MSG_GameMsgDownHead
{
	CT_DWORD		dwUserID;
	CT_LONGLONG		llScore;											//玩家金币
	CT_DWORD        dwWinCount;											//赢得次数
	CT_DWORD        dwLostCount;										//输的次数
	CT_DWORD        dwDrawCount;										//和的次数
	CT_DWORD        dwFleeCount;										//逃跑次数
	CT_BYTE			cbSex;												//性别
	CT_BYTE			cbVipLevel;											//VIP等级
};

//查询红包信息
struct MSG_SC_Query_UserHongBaoRecord_Info 
{
	CT_BYTE			cbType;												//0：同意 1：猜中 2：回收
	CT_CHAR			szHongBaoName[HONGBAO_NAME_LEN];					//红包名称
	CT_CHAR			szNickName[NICKNAME_LEN];							//玩家昵称
	CT_DWORD		dwHongBaoScore;										//红包金额
};

//查询玩家游戏记录返回
struct MSG_SC_Query_GameRecord_Head
{
	CT_WORD 		wGameID;								//游戏ID
	CT_WORD 		wKindID;								//类型ID
};

struct MSG_SC_Query_GameRecord
{
	CT_DWORD		dwDrawID;								//游戏记录ID
    CT_WORD 		wRoomKindID;							//房间类型ID
	CT_WORD			wSelfChairID;							//玩家的椅子ID
	CT_BYTE         cbReport;								//玩家是否已经举报
	CT_DWORD 		dwBankerUserID;							//庄家的玩家ID
	CT_BYTE			cbUserCount;							//游戏人数
	CT_DWORD		arrUserID[5];							//用户ID
	CT_DOUBLE		arrScore[5];							//用户输赢分
	CT_CHAR			szDate[DATE_LEN];						//日期
	CT_CHAR			szTime[TIME_LEN_2];						//时间
};

//红包扫雷游戏记录
struct MSG_SC_Query_HBSLRecord
{
	CT_DWORD        dwSendUserId;                           //发送者ID
	CT_DWORD 		dwSendTime;								//发红包时间
	CT_BYTE         cbHeadID;                               //头像ID
	CT_BYTE         cbSex;                                  //性别
	CT_BYTE         cbVip2;                                 //VIP2等级
	CT_DOUBLE       dAmount;                                //红包金额
    CT_WORD         wAllotCount;		                    //分包个数
    CT_DOUBLE       dMultiple;                             //红包赔率
    CT_BYTE         cbThunderNO;                            //雷号
};

struct MSG_SC_Query_HBSLRecord_Detail
{
    CT_DWORD        dwUserID;                               //玩家ID
    CT_DOUBLE       dGrabScore;                             //抢到的红包金额
    CT_BYTE         cbIsThunder;                            //是否中累
};

//红包扫雷彩金记录
struct MSG_SC_Query_HBSLWinColorPrizeRecord
{
	CT_DWORD        dwUserID;                               //玩家ID
	CT_BYTE         cbSex;                                  //性别
	CT_BYTE         cbVip2;                                 //VIP2等级
	CT_BYTE         cbHeadID;                               //头像ID
	CT_DOUBLE       dWinColorPrize;                         //中彩金额
	CT_BYTE         cWinPrizeType;                          //抢红包获得彩金1，发红包获得彩金为2
    CT_DWORD 		dwTime;								    //时间
};
/////////////////////////////////////////////////////////////////////////////////////
//红包相关
#define		MSG_HONGBAO_MAIN							21
#define		SUB_CS_QUERY_HONGBAO_INFO					21001			//查询所有红包信息【MSG_CS_Query_HongBao】
#define		SUB_CS_SEND_HONGBAO							21002			//发送红包【MSG_CS_Send_HongBao】
#define		SUB_CS_GUESS_HONGBAO						21003			//猜红包【MSG_CS_Guess_HongBao】
#define		SUB_CS_QUERY_MYSELF_HONGBAO					21004			//查询本人红包信息【MSG_CS_Query_Myself_HongBao】
#define		SUB_CS_OPERATE_GUESSOK_HONGBAO				21005			//操作猜中的红包【MSG_CS_Operate_GuessOk_HongBao】
#define		SUB_CS_TAKE_TIMEOUT_HONGBAO					21006			//领回超时红包【MSG_CS_Take_TimeOut_HongBao】

#define		SUB_SC_HONGBAO_COUNT						21050			//红包数目【MSG_SC_HongBao_Count】
#define		SUB_SC_HONGBAO_INFO							21051			//红包信息【一组MSG_SC_HongBao_Info】
#define		SUB_SC_GUESS_OK_HONGBAO						21052			//属于本人并被猜中的红包【MSG_SC_Guess_OK_HongBao】
#define		SUB_SC_TIMEOUT_HONGBAO						21053			//属于本人并超时的红包【MSG_SC_TimeOut_HongBao】
#define		SUB_SC_HONGBAO_INFO_FINISH					21054			//红包发送完成【无】
#define		SUB_SC_SEND_HONGBAO							21055			//发送红包【MSG_SC_Send_HongBao】
#define		SUB_SC_GUESS_HONGBAO						21056			//猜红包返回【MSG_SC_Guess_HongBao】
#define		SUB_SC_QUERY_MYSELF_HONGBAO					21057			//查询本人红包信息返回【MSG_SC_Query_Myself_HongBao】
#define		SUB_SC_OPERATE_GUESSOK_HONGBAO				21058			//操作猜中的红包返回【MSG_SC_Operate_GuessOK_HongBao】
#define		SUB_SC_TAKE_TIMEOUT_HONGBAO					21059			//领回超时红包返回【MSG_SC_Take_TimeOut_HongBao】

struct MSG_CS_Query_HongBao
{
	CT_DWORD		dwUserID;							//玩家ID
};

struct MSG_CS_Send_HongBao
{
	CT_DWORD		dwUserID;							//发送者ID
	CT_DWORD		dwScore;							//发送金额
	CT_CHAR			szName[HONGBAO_NAME_LEN];			//红包名称
};

struct MSG_CS_Guess_HongBao
{
	CT_DWORD		dwUserID;							//猜测者ID
	CT_DWORD		dwHongBaoID;						//红包ID
	CT_DWORD		dwGuessScore;						//猜红包数额
};

struct MSG_CS_Query_Myself_HongBao
{
	CT_DWORD		dwUserID;							//用户ID
	CT_DWORD		dwHongBaoID;						//红包ID
};

struct MSG_CS_Operate_GuessOk_HongBao
{
	CT_DWORD		dwUserID;							//红包所有者ID
	CT_DWORD		dwHongBaoID;						//红包ID
	CT_BYTE			cbOperateCode;						//操作码（0：不同意 1：同意）
};

struct MSG_CS_Take_TimeOut_HongBao
{
	CT_DWORD		dwUserID;							//用户ID
	CT_DWORD		dwHongBaoID;						//红包ID
};

struct MSG_SC_HongBao_Count : public MSG_GameMsgDownHead
{
	CT_DWORD		dwCount;							//红包数量
};

struct MSG_SC_HongBao_Info
{
	CT_DWORD		dwHongBaoID;						//红包ID
	CT_DWORD		dwSenderUserID;						//发送者ID
	CT_CHAR			szSenderNickName[NICKNAME_LEN];		//发送者昵称
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];			//发送者头像
	CT_CHAR			szHongBaoName[HONGBAO_NAME_LEN];	//红包名称
};
 
struct MSG_SC_Guess_OK_HongBao
{
	CT_DWORD		dwHongBaoID;						//红包ID
	CT_CHAR			szHongBaoName[HONGBAO_NAME_LEN];	//红包名称
	CT_DWORD		dwGuessOkUserID;					//猜中玩家ID
	CT_CHAR			szGuessOkNickName[NICKNAME_LEN];	//猜中者昵称
	CT_DWORD		dwHongBaoScore;						//红包金额
}; 

struct MSG_SC_TimeOut_HongBao
{
	CT_DWORD		dwHongBaoID;						//红包ID
	CT_CHAR			szHongBaoName[HONGBAO_NAME_LEN];	//红包名称
	CT_DWORD		dwHongBaoScore;						//红包金额
};

struct MSG_SC_Send_HongBao : public MSG_GameMsgDownHead
{
	CT_BYTE					cbResult;					//结果(0：成功 1：金币不足 2：在游戏中不能发送红包）

	MSG_SC_HongBao_Info		hongbaoInfo;				//新增红包信息
};

struct MSG_SC_Guess_HongBao : public MSG_GameMsgDownHead
{
	CT_BYTE					cbResult;					//结果(0： 成功 1：红包不存在或者已经被别人领取走了 2：口令错误  3：红包已过期)
	CT_DWORD				dwHongbaoID;				//红包ID
	CT_DWORD				dwHongBaoScore;				//红包金额
};

struct MSG_SC_Query_Myself_HongBao : public MSG_GameMsgDownHead
{
	CT_BYTE					cbResult;					//查询结果（0：成功 1：红包不存在或者已经被领取 2：这个红包不属于你的）
	CT_WORD					wGuessCount;				//竞猜次数
	CT_DWORD				dwHongBaoID;				//红包ID
	CT_DWORD				dwHongBaoScore;				//红包金额
	CT_DWORD				dwRemainTime;				//红包剩余时间
};

struct MSG_SC_Operate_GuessOK_HongBao : public MSG_GameMsgDownHead
{
	CT_BYTE					cbResult;					//操作结果（0：成功 1：操作失败）
	CT_DWORD				dwHongBaoID;				//红包ID
};

struct MSG_SC_Take_TimeOut_HongBao : public MSG_GameMsgDownHead
{
	CT_BYTE					cbResult;					//操作结果（0：成功 1：操作失败）
	CT_DWORD				dwHongBaoID;				//红包ID
};

/////////////////////////////////////////////////////////////////////////////////////
//排行榜相关
#define		MSG_RANK_MAIN							22

#define		SUB_CS_TODAY_EARN_SCORE_RANK			22001			//查询今天赚金榜
#define 	SUB_CS_TODAY_ONLINE_RANK				22002			//今日在线榜
#define 	SUB_CS_TODAY_TASK_RANK					22003			//今日任务榜

#define		SUB_SC_SCORE_RANK						22050			//金币排行榜【一组MSG_SC_Score_Rank】
#define		SUB_SC_TODAY_EARN_SCORE_RANK			22051			//今日赚金排行榜【一组MSG_SC_Score_Rank】
#define		SUB_SC_SIGNIN_RANK						22052			//签到榜【一组MSG_SC_SignIn_Rank】
#define		SUB_SC_SHARE_SCORE_RANK					22053			//分享榜【一组MSG_SC_Share_Rank】
#define 	SUB_SC_TODAY_ONLINE_RANK				22054			//今日在线榜【一组MSG_SC_Online_Rank】
#define 	SUB_SC_TODAY_TASK_RANK					22055			//今日任务榜【一组MSG_SC_Task_Rank】

//查询今日赚金榜
struct MSG_CS_Query_Score_Rank
{
	CT_DWORD		dwUserID;						//用户ID
};

//金币排行榜
struct MSG_SC_Score_Rank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_DOUBLE		dScore;							//玩家金币
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//发送者名称
};

//今日在线排行榜
struct MSG_SC_Online_Rank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_DWORD		dwOnline;						//在线时长
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//玩家昵称
};

//今日任务榜
struct MSG_SC_Task_Rank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_WORD			wFinishCount;					//任务完成次数
	CT_DOUBLE 		dRewardScore;					//奖励金币
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//玩家昵称
};

//签到榜
struct MSG_SC_SignIn_Rank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_BYTE			cbVipLevel;						//VIP等级
	CT_WORD			wSignInCount;					//连续签到次数
	CT_CHAR			szNickName[NICKNAME_LEN];		//用户名称
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];		//用户头像
};

//分享排行榜
struct MSG_SC_Share_Rank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_DWORD		dwShareCount;					//分享次数
	CT_DOUBLE		dRewardScore;					//奖励金币
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//发送者名称
};

/////////////////////////////////////////////////////////////////////////////////////
//任务相关
#define		MSG_TASK_MAIN							23

#define		SUB_CS_GET_TASK_REWARD					23001			//领取任务奖励【MSG_CS_Get_TaskReward】

#define		SUB_SC_TASK_INFO						23049			//任务信息【一组MSG_SC_Task_Info】
#define		SUB_SC_TASK_PROGRESS					23050			//任务进度【一个或者一组MSG_SC_Task_Progress】
#define		SUB_SC_TASK_REWARD_RESULT				23051			//领取任务奖励结果【MSG_SC_Task_Reward_Result】

//领取任务奖励
struct MSG_CS_Get_TaskReward
{
	CT_DWORD		dwUserID;						//用户ID
	CT_WORD			wTaskID;						//任务ID
};

//任务信息
struct MSG_SC_Task_Info 
{
	CT_WORD			wTaskID;						//任务ID
	CT_WORD			wTaskType;						//任务类型
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//子游戏ID
	CT_WORD			wRoomKindID;					//房间类型
	CT_DWORD		dwTotalProgress;				//任务总进度
	CT_DWORD		dwReward;						//任务奖励
};

//更新任务进度
struct MSG_SC_Task_Progress
{
	CT_WORD			wTaskID;						//任务ID
	CT_BYTE			cbFinish;						//任务状态(0:未完成 1:已完成)
	CT_DWORD		dwCurrProgress;					//当前进度
	CT_DWORD		dwTotalProgress;				//总进度
};

//领取任务奖励结果
struct MSG_SC_Task_Reward_Result : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;						//任务结果(0:成功 1:任务未达到领取条件)
	CT_WORD			wTaskID;						//任务ID
	CT_DWORD		dwReward;						//任务奖励
};

/////////////////////////////////////////////////////////////////////////////////////
//广播消息相关
#define		MSG_BROADCAST_MSG_MAIN					24

#define		SUB_SC_GAME_MSG							24050		//游戏消息【MSG_SC_Broadcast_GameMsg】
#define		SUB_SC_SYS_MSG							24051		//系统消息【MSG_SC_Broadcast_SysMsg】

struct MSG_SC_Broadcast_GameMsg : public MSG_GameMsgDownHead
{
	CT_CHAR			szNickname[NICKNAME_LEN];					//玩家昵称
	CT_CHAR			szServerName[SERVER_NAME_LEN];				//玩家所有房间
	CT_WORD 		wGameID;									//gameid *100 + kindid
  	CT_BYTE 		cbVip2;										//Vip等级
  	CT_BYTE 		cbCardType;									//牌型
	CT_DOUBLE		dWinScore;									//赢的金币
};

struct MSG_SC_Broadcast_SysMsg : public MSG_GameMsgDownHead
{
	CT_CHAR			szSysMsg[SYSTEM_MSG_LEN];
};

/////////////////////////////////////////////////////////////////////////////////////
//充值渠道、兑换、公告等相关配置信息
#define		MSG_CONFIG_MSG_MAIN				    25

#define		SUB_C2S_REMOVE_RECHARGE_INFO		25001		//删除充值提醒消息【MSG_CS_Remove_RechargeInfo】
#define     SUB_C2S_QUERY_CHATUPLOAD_URL        25002       //查询聊天图片上传地址【MSG_CS_ChatUpload_Url】
#define     SUB_C2S_QUERY_ANNOUNCEMENT_CONTENT  25003       //查询公告的内容【MSG_CS_AnnouncementContent】


#define		SUB_S2C_RECHARGE_CHANNEL_INFO_MSG	25050		//充值渠道信息【MSG_SC_RECHARGE_CHANNEL_INFO】
#define		SUB_S2C_RECHARGE_TIPS_INFO			25051		//充值提醒信息【MSG_SC_RechargeInfo】
#define		SUB_S2C_RECHARGE_PROMOTER_LIST		25052		//代理充值信息【MSG_SC_PromoterInfo】
#define		SUB_S2C_GENERALIZE_PROMOTER			25053		//推广代理信息(全民代理)【MSG_SC_GeneralizePromoter_Info】
#define 	SUB_S2C_GENERALIZE_PROMOTER_STATUS	25054		//推广代理按钮状态【MSG_SC_GeneralizePromoter_Status】
#define 	SUB_S2C_PLATFORM_RECHARGE_CFG		25055		//跟平台相关的充值配置【MSG_SC_PlatformRechargeCfg】

#define		SUB_S2C_EXCHANGE_CHANNEL_STATUS		25060		//兑换渠道的状态
#define		SUB_S2C_CLIENT_CHANNEL_DOMAIN		25061		//客户端渠道的主页
#define     SUB_S2C_QUERY_CHATUPLOAD_URL        25062       //查询聊天图片上传地址【MSG_SC_ChatUpload_Url】
#define 	SUB_S2C_ANNOUNCEMENT_INFO			25063		//公告的信息【一组MSG_SC_Announcement_Info】
#define     SUB_S2C_ANNOUNCEMENT_CONTENT        25064       //公告的内容【MSG_SC_Announcement_Content】


//删除充值提醒消息
struct MSG_CS_Remove_RechargeInfo
{
	CT_DWORD		dwUserID;						//用户ID
};

//查询聊天图片上传地址
struct MSG_CS_ChatUpload_Url
{
    CT_DWORD		dwUserID;						//用户ID
};

//查询公告的内容
struct MSG_CS_AnnouncementContent
{
    CT_DWORD        dwUserID;                       //用户ID
    CT_BYTE         cbAnnouncementID;               //公告ID
};

//充值渠道信息
struct MSG_SC_Recharge_Channel_Info : public MSG_GameMsgDownHead
{
	CT_BYTE		cbChannelID[RECHARGE_COUNT];				//客户端按照数组顺序显示(渠道ID：1微信 2支付宝 3网银 4QQ钱包 5代理 6云闪付 7定额支付宝 8定额微信)
	CT_BYTE		cbState[RECHARGE_COUNT];					//对应的渠道状态(0：禁用 1：启用)   //old（0：禁用 1：启用定额 2：启用非定额 3：启用定额和非定额）
	CT_DWORD 	dwAmount[RECHARGE_COUNT][7];				//渠道对应的充值金额(0-6是金额，7是最大充值金额)
};

//充值提醒消息
struct MSG_SC_RechargeInfo :public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;									//充值结果（0: 充值失败 1：充值成功）
	CT_LONGLONG		llRecharge;									//成功充值金额
};

//代理充值信息
struct MSG_SC_PromoterInfo : public MSG_GameMsgDownHead
{
	CT_DWORD		dwListLen;									//长度
	CT_CHAR			szPromoterList[1];							//代理列表
};

//平台相关的充值配置信息
struct MSG_SC_PlatformRechargeCfg : public MSG_GameMsgDownHead
{
	CT_DWORD		dwListLen;									//长度
	CT_CHAR			szJsonCfg[1];								//配置Json
};

//兑换帐号状态
struct MSG_SC_Exchange_Channel_Info : public MSG_GameMsgDownHead
{
	CT_BYTE			cbChannelStatus[2];							//渠道状态(cbChannelStatus[0]表示是支付宝状态，cbChannelStatus[1]表示是银联状态)
	CT_DWORD 		dwAmount[2][5];								//渠道对应的兑换金额
    CT_WORD 	    dwMinAmount[2];			                    //兑换最小金额(0表示没有限制)
    CT_DWORD 	    dwMaxAmount[2];			                    //兑换最大金额(0表示没有限制)
    CT_BYTE		    cbDayCountLimite[2];		                //每天兑换次数限制(0表示没有限制)
    CT_WORD 	    wHandlingFee[2];			                //单笔手续费(千分比)
};

//推广代理信息
struct MSG_SC_GeneralizePromoter_Info : public MSG_GameMsgDownHead
{
	CT_BYTE		cbPromoterCount;
	CT_CHAR		szAccount[3][NICKNAME_LEN];
	CT_CHAR		szNickName[3][NICKNAME_LEN];
	CT_BYTE		cbType[3];
};

//推广代理的状态
struct MSG_SC_GeneralizePromoter_Status : public MSG_GameMsgDownHead
{
	CT_BYTE		cbStatus;
};

//客户端渠道的主页
struct MSG_SC_ClientChannel_Domain : public MSG_GameMsgDownHead
{
	CT_CHAR		szChannelDomain[CHANNEL_DOMAIN_LEN];
	CT_CHAR		szChannelDomain2[CHANNEL_DOMAIN_LEN];
};

//聊天上传图片地址
struct MSG_SC_ChatUpload_Url : public MSG_GameMsgDownHead
{
	CT_WORD		wUrlLen;									//长度
	CT_CHAR 	szUploadUrl[1];								//地址内容http://test.upload.lerengu.com
};

//公告标题信息
struct MSG_SC_Announcement_Info
{
	CT_BYTE		cbAnnouncementID;                           //公告ID
	CT_CHAR     szAnnounceName[ANNOUNTCEMENT_TITLE_LEN];    //公告标题
};

//公告内容
struct MSG_SC_Announcement_Content
{
    CT_BYTE     cbAnnouncementID;                           //公告ID
    CT_DWORD    dwContentLen;                               //内容长度
    CT_CHAR     szContent[1];                               //公告内容
};

/////////////////////////////////////////////////////////////////////////////////////
//财神降临相关
#define		MSG_WEALTHGODCOMING_MAIN					26

#define		SUB_CS_WEALTHGODCOMING_MAIN_INFO			26000			//查询财神降临活动主面板信息【MSG_CS_WealthGodComing_Main】
#define 	SUB_CS_WEALTHGODCOMING_REWARD_RULE          26001           //奖励规则【MSG_CS_WealthGodComing_Reward_Rule】
#define     SUB_CS_WEALTHGODCOMING_DETAIL               26002           //活动详情【MSG_CS_WealthGodComing_Detail】
#define     SUB_CS_WEALTHGODCOMING_REWARD_LIST          26003           //获奖名单【MSG_CS_WealthGodComing_Reward_List】

#define     SUB_SC_WEALTHGODCOMING_MAIN_INFO            26050           //财神降临活动主面板信息【一组MSG_SC_WealthGodComing_MainInfo】
#define     SUB_SC_WEALTHGODCOMING_REWARD_RULE          26051           //奖励规则【一组MSG_SC_WealthGodComing_RewardRule】
#define     SUB_SC_WEALTHGODCOMING_DETAIL               26052           //活动详情【一组MSG_SC_WealthGodComing_Detail】
#define     SUB_SC_WEALTHGODCOMING_REWARD_LIST          26053           //获奖名单【一组MSG_SC_WealthGodComing_RewardList】
#define 	SUB_SC_CURRENT_WEALTHGODCOMING_ACTIVITY		26054			//当前正在进行的财神活动[一组MSG_SC_Current_WealthGodComing]
#define     SUB_SC_WEALTHGODCOMING_REAL_TIME_INFO       26055           //财神降临的实时信息(排名和倒计时)【MSG_SC_WealthGodComing_RealTimeInfo】
#define     SUB_SC_WEALTHGODCOMING_END                  26056           //财神任务结束[MSG_SC_WealthGodComing_End]
#define     SUB_SC_WEALTHGODCOMING_USER_RANK            26057           //财神个人名次[MSG_SC_WealthGodComing_UserRank]

//查询财神降临主信息
struct MSG_CS_WealthGodComing_Main
{
	CT_DWORD 	dwUserID;								//用户ID
};

//查询奖励规则
struct MSG_CS_WealthGodComing_Reward_Rule
{
	CT_DWORD 	dwUserID;                               //用户ID
	CT_WORD		wActivityID;							//活动ID
};

//查询活动详情
struct MSG_CS_WealthGodComing_Detail
{
	CT_DWORD 	dwUserID;								//用户ID
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//游戏类型ID
	CT_WORD 	wRoomKind;								//房间类型
};

//查询活动获奖名单
struct MSG_CS_WealthGodComing_Reward_List
{
	CT_DWORD 	dwUserID;								//用户ID
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//游戏类型ID
	CT_WORD 	wRoomKind;								//房间类型
	CT_DWORD    dwIndex;                                //场次ID
};

//财神降临主面板活动信息
struct MSG_SC_WealthGodComing_MainInfo
{
    CT_WORD     wActivityID;                            //活动ID
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//游戏类型ID
	CT_WORD 	wRoomKind;								//房间类型
	CT_CHAR		szTime[TIME_LEN_3];						//活动时间
	CT_BYTE 	cbStatus;								//活动状态（0: 未开始 1：正在进行 2：已结束）
};

struct MSG_SC_WealthGodComing_MainInfoEx : public MSG_SC_WealthGodComing_MainInfo
{
	CT_WORD 	wStartTime;								//开始时间
};

//奖励规则信息
struct MSG_SC_WealthGodComing_RewardRule
{
    CT_BYTE     cbRank;                                 //名次
    CT_BYTE     cbRankStandar;                          //排名标准（1 输赢 2 下注）
    CT_WORD     wCompleteCount;                         //完成次数
    CT_DOUBLE   dReward;                               	//奖励
};

//活动详情
struct MSG_SC_WealthGodComing_Detail
{
    CT_DWORD    dwIndex;                                //场次ID
	CT_CHAR		szDate[DATE_LEN];						//活动日期
    CT_CHAR     szTime[TIME_LEN_3];                     //活动时间
    CT_BYTE     cbSelfJoin;                             //本人是否参与
    CT_BYTE     cbStatus;                               //活动状态（0: 未开始 1：正在进行 2：已发奖 3：活动失败）
};

//获奖名单
struct MSG_SC_WealthGodComing_RewardList
{
	CT_BYTE 	cbRank;									//名次
	CT_DWORD 	dwUserID;								//用户ID
	CT_BYTE 	cbGender;								//性别
	CT_BYTE		cbHeadID;								//头像ID
	CT_BYTE		cbVip2;									//VIP2
	CT_DOUBLE 	dScore;									//用户成绩
	CT_DOUBLE 	dReward;								//奖励
	CT_WORD 	wValidCount;							//有效局数
	CT_WORD 	wDutyCompleteCount;						//责任局数
};

//当前进行的财神活动
struct MSG_SC_Current_WealthGodComing
{
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//游戏类型ID
	CT_WORD 	wRoomKindID;							//房间类型ID
};

//财神活动的实时信息
struct MSG_SC_WealthGodComing_RealTimeInfo : public MSG_GameMsgDownHead
{
    CT_WORD 	wGameID;								//游戏ID
    CT_WORD 	wKindID;								//游戏类型ID
    CT_WORD 	wRoomKindID;							//房间类型ID
    CT_DWORD    dwTimeLeft;                             //剩余时间
    CT_WORD     nRank;                                  //用户排名
    CT_WORD     nTotalUser;                             //参总用户
};

//财神活动的实时信息
struct MSG_SC_WealthGodComing_End : public MSG_GameMsgDownHead
{
    CT_WORD 	wGameID;								//游戏ID
    CT_WORD 	wKindID;								//游戏类型ID
    CT_WORD 	wRoomKindID;							//房间类型ID
};

//财神活动的名次
struct MSG_SC_WealthGodComing_UserRank : public MSG_GameMsgDownHead
{
    CT_BYTE     cbRank;                                 //名次
};

//一元夺宝相关
#define		MSG_DUOBAO_MAIN								27

#define		SUB_CS_DUOBAO_MAIN_INFO						27000			//查询夺宝主面板信息【MSG_CS_DuoBao_Main】
#define 	SUB_CS_QUERY_SELF_LUCKY_NUM					27001			//查询自己的某活动某期的幸运码【MSG_CS_Query_Self_DuoBao_LuckyNum】
#define     SUB_CS_QUERY_SELF_HISTORY			        27002           //查看自己历史记录【MSG_CS_Query_Self_DuoBao_History】
#define     SUB_CS_QUERY_HISTORY_LUCKY_USER             27003           //查询某活动往期幸运儿【MSG_CS_Query_DuoBao_History_LuckyUser】
#define     SUB_CS_BUY_LUCKY_NUM                        27004           //购买幸运码【MSG_CS_Buy_LuckyNum】
#define     SUB_CS_CLOSE_UI                             27005           //关闭主面板【MSG_CS_Close_DuoBao_UI】
#define 	SUB_CS_QUERY_REWARD_RECORD					27006			//查询获奖记录【MSG_CS_Query_Duobao_Reward_Record】

#define     SUB_SC_DUOBAO_MAIN_INFO            			27050           //财神降临活动主面板信息【一组MSG_SC_DuoBao_Main】
#define 	SUB_SC_QUERY_SELF_LUCKY_NUM					27051			//查询自己的某活动某期的幸运码【MSG_SC_Query_Self_DuoBao_LuckyNum+一组CT_DWORD】
#define     SUB_SC_QUERY_SELF_HISTORY       			27052           //查看自己历史记录【一组MSG_SC_Query_Self_DuoBao_History】
#define     SUB_SC_QUERY_HISTORY_LUCKY_USER             27053           //查询某活动往期幸运儿【一组MSG_SC_Query_DuoBao_History_LuckyUser】
#define     SUB_SC_BUY_LUCKY_NUM                        27054           //购买幸运码【MSG_SC_Buy_LuckyNum】
#define     SUB_SC_UPDATE_LUCKYNUM_COUNT                27055           //更新幸运码的数量【MSG_SC_Update_LuckyNum_Count】（只对打开UI的用户更新）
#define     SUB_SC_UPDATE_DUOBAO_STATE                  27056           //更新活动的状态【MSG_SC_Update_DuoBao_State】（只对打开UI的用户更新）
#define     SUB_SC_QUERY_REWARD_RECORD                  27057           //查询获奖记录【一组MSG_SC_Query_DuoBao_Reward_Record】

//C->S
//查询夺宝主信息
struct MSG_CS_DuoBao_Main
{
	CT_DWORD	dwUserID;               //用户ID
};

//查询自己的幸运码
struct MSG_CS_Query_Self_DuoBao_LuckyNum
{
    CT_DWORD    dwUserID;               //用户ID
    CT_WORD     wID;                    //夺宝ID
    CT_DWORD    dwIndex;                //夺宝第几期
};

//查询自己的夺宝记录
struct MSG_CS_Query_Self_DuoBao_History
{
    CT_DWORD    dwUserID;               //用户ID
};

//查询某活动往期幸运儿
struct MSG_CS_Query_DuoBao_History_LuckyUser
{
    CT_DWORD    dwUserID;               //用户ID
    CT_WORD     wID;                    //夺宝ID
};

//查询获奖记录
struct MSG_CS_Query_Duobao_Reward_Record
{
	CT_DWORD 	dwUserID;				//用户ID
};

//购买幸运码
struct MSG_CS_Buy_LuckyNum
{
    CT_DWORD    dwUserID;               //用户ID
    CT_WORD     wID;                    //夺宝ID
    CT_DWORD    dwIndex;                //夺宝第几期
    CT_DWORD    dwBuyCount;             //购买次数
};

//关闭夺宝UI
struct MSG_CS_Close_DuoBao_UI
{
    CT_DWORD    dwUserID;
};

//S->C
//查询夺宝主信息返回
struct MSG_SC_DuoBao_Main
{
	CT_WORD		wID;				    //夺宝ID
	CT_DWORD    dwIndex;                //当前第几期
	CT_DWORD 	dwUnitPrice;		    //单价
    CT_DWORD    dwUserLeaseScore;       //玩家购买至少需要金额
    CT_DWORD 	dwBuyCount;			    //已经购买次数
	CT_DWORD 	dwSelfBuyCount; 	    //自己购买次数
	CT_WORD     wlimitCount;            //个人限制次数
	CT_DWORD    dwTotalCount;           //总次数
	CT_DWORD 	dwRewardScore;		    //奖励金币
	CT_CHAR     szTitle[DUOBAO_TITLE];  //夺宝标题
	CT_BYTE     cbState;                //状态(0表示没有开奖 1表示正在开奖 2表示已经开奖 3表示过期不开奖)
};

//查询幸运码返回
struct MSG_SC_Query_Self_DuoBao_LuckyNum
{
    CT_WORD		wID;				    //夺宝ID
    CT_DWORD    dwIndex;                //当前第几期
};

//查询自己的夺宝记录返回
struct MSG_SC_Query_Self_DuoBao_History
{
    CT_WORD     wID;                    //夺宝ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD 	dwBuyCount;			    //已经购买次数
    CT_DWORD 	dwSelfBuyCount; 	    //自己购买次数
    CT_DWORD    dwTotalCount;           //总次数
    CT_DWORD    dwEndTime;              //结束时间戳
    CT_DWORD    dwLuckyNum;             //幸运码
    CT_DWORD    dwLuckyUser;            //幸运儿
    CT_BYTE     cbLuckySex;             //幸运儿性别
    CT_BYTE     cbLuckyHeadIndex;       //幸运儿头像索引
    CT_CHAR 	szTitle[DUOBAO_TITLE];  //夺宝标题
    CT_BYTE     cbState;                //状态(0表示没有开奖 1表示正在开奖 2表示已经开奖 3表示过期不开奖)
};

//查询某活动往期幸运儿返回
struct MSG_SC_Query_DuoBao_History_LuckyUser
{
    CT_WORD     wID;                    //活动ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD    dwEndTime;              //结束时间戳
    CT_DWORD    dwLuckyUser;            //幸运儿
    CT_DWORD    dwLuckyUserBuyCount;    //幸运儿参加次数
    CT_DWORD    dwLuckyNum;             //幸运码
    CT_BYTE     cbLuckySex;             //幸运儿性别
    CT_BYTE     cbLuckyHeadIndex;       //幸运儿头像索引
};

//查询获奖记录
struct MSG_SC_Query_DuoBao_Reward_Record
{
	CT_WORD     wID;                    //夺宝ID
	CT_DWORD    dwIndex;                //第几期
//	CT_DWORD 	dwBuyCount;			    //已经购买次数
	CT_DWORD 	dwSelfBuyCount; 	    //自己购买次数
//  CT_DWORD    dwTotalCount;           //总次数
	CT_DWORD    dwEndTime;              //结束时间戳
	CT_DWORD    dwLuckyNum;             //幸运码
//  CT_DWORD    dwLuckyUser;            //幸运儿
//  CT_BYTE     cbLuckySex;             //幸运儿性别
//  CT_BYTE     cbLuckyHeadIndex;       //幸运儿头像索引
	CT_CHAR 	szTitle[DUOBAO_TITLE];  //夺宝标题
};

//购买幸运码
struct MSG_SC_Buy_LuckyNum : public MSG_GameMsgDownHead
{
    CT_BYTE     cbResult;               //购买结果(0成功 1金币不足 2活动已经结束 3购买次数超过剩余次数 4在其他游戏中不能购买幸运码 5参与夺宝后身上需要保留多少钱)
    CT_WORD     wID;                    //活动ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD    dwBuyCount;             //购买成功返回总次数
};

//购买幸运码
struct MSG_SC_Update_LuckyNum_Count : public MSG_GameMsgDownHead
{
    CT_WORD     wID;                    //活动ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD    dwBuyCount;             //当前购买的总数
};

//更新状态
struct MSG_SC_Update_DuoBao_State : public  MSG_GameMsgDownHead
{
    CT_BYTE     cbState;                //状态(0新开一场活动 1表示正在开奖 2表示已经开奖 3表示过期不开奖)当状态为2时,附带幸运儿的ID和头像，幸运码
    CT_WORD     wID;                    //活动ID
    CT_DWORD    dwIndex;                //第几期
    CT_DWORD 	dwLuckyUserID;			//幸运儿ID
    CT_DWORD 	dwLuckyNum;				//幸运码
    CT_BYTE 	cbSex;					//幸运儿性别
    CT_BYTE 	cbHeadIndex;			//幸运儿头像ID
};

/////////////////////////////////////////////////////////////////////////////////////
//新年红包相关
#define		MSG_REDPACKET_MAIN							28

#define		SUB_CS_REDPACKET_MAIN_INFO					28001			//查询红包主面板信息【MSG_CS_RedPacket_Main】
#define 	SUB_CS_REDPACKET_GRAB_LUCKY_MONEY           28002           //抢红包【MSG_CS_RedPacket_Grab_LuckyMoney】

#define 	SUB_SC_REDPACKET_STATUS						28050			//红包系统状态【MSG_SC_RedPacket_Status】
#define     SUB_SC_REDPACKET_MAIN_INFO            		28051           //红包主面板信息【MSG_SC_RedPacket_Main和一组MSG_SC_RedPacket_Room】
#define     SUB_SC_REDPACKET_GRAB_LUCKY_MONEY           28052           //抢红包的返回【MSG_SC_RedPacket_Grab_LuckyMoney】
#define     SUB_SC_REDPACKET_GAMEROOM_STATUS            28053           //红包活动状态【MSG_SC_RedPacket_GameRoom_Status和一组MSG_SC_RedPacket_Room】
#define     SUB_SC_REEPACKET_START                      28054           //红包活动开始【MSG_SC_RedPacket_Start】

//C->S
//查询红包主信息
struct MSG_CS_RedPacket_Main
{
	CT_DWORD 	dwUserID;								//用户ID
};

//抢红包
struct MSG_CS_RedPacket_Grab_LuckyMoney
{
	CT_DWORD 	dwUserID;                               //用户ID
	CT_DWORD    dwRedPacketIndex;                       //红包期数
	CT_WORD     wGameID;                                //游戏ID
	CT_WORD     wKindID;                                //游戏类型
	CT_WORD     wRoomKindID;                            //房间类型
};

//S->C
//红包系统状态(是指红包活动是否开启,登录时发送)
struct MSG_SC_RedPacket_Status : public MSG_GameMsgDownHead
{
    CT_BYTE     cbStatus;                               //状态(0:未开放 1:开放弹窗口 2:开放不弹窗口)
	CT_CHAR     szDes[RED_PACKET_DES];                  //红包说明
};

//红包主界面
struct MSG_SC_RedPacket_Main
{
    MSG_SC_RedPacket_Main()
    {
        memset(this, 0, sizeof(*this));
    }
	CT_CHAR 	szStartDate[DATE_LEN];					//开始日期
	CT_CHAR     szStartTime[TIME_LEN_2];                //开始时间
	//CT_CHAR     szDes[RED_PACKET_DES];                  //红包说明
	CT_DWORD    dwLastIndex;                            //上一期ID
	CT_BYTE     cbBigSex[RED_PACKET_BIGWINNER];         //上一期大赢家性别
	CT_BYTE     cbBigHeadId[RED_PACKET_BIGWINNER];      //上一期大赢头像
	CT_BYTE     cbVip2[RED_PACKET_BIGWINNER];           //上一期大赢VIP2
	CT_DWORD	dwBigWiner[RED_PACKET_BIGWINNER];       //上一期大赢家
	CT_DOUBLE   dBigScore[RED_PACKET_BIGWINNER];        //上一期大赢家金币
	CT_BYTE     cbGameRoomCount;                        //正在进行或将进行的红包活动房间数量
};
struct MSG_SC_RedPacket_Room
{
    MSG_SC_RedPacket_Room()
    :wGameID(0)
    ,wKindID(0)
    ,wRoomKindID(0)
    {}
    CT_WORD     wGameID;
    CT_WORD     wKindID;
    CT_WORD     wRoomKindID;
};

//抢红包返回
struct MSG_SC_RedPacket_Grab_LuckyMoney : public MSG_GameMsgDownHead
{
    CT_BYTE     cbResult;                               //结果(1: 成功 2: 未达到领取红包的资格 3:红包已经全部抢完 4:之前已经抢到红包)
    CT_DOUBLE   dLuckyMoney;                            //抢到的红包金额
};

//红包活动状态
struct MSG_SC_RedPacket_GameRoom_Status : public MSG_GameMsgDownHead
{
	MSG_SC_RedPacket_GameRoom_Status()
	:dwRedPacketIndex(0)
	,cbStatus(2)
	{}
    CT_DWORD    dwRedPacketIndex;                       //红包期数
    CT_BYTE     cbStatus;                               //状态(1: 正在进行 2: 红包活动结束)
};

//红包活动开始
struct MSG_SC_RedPacket_Start : public MSG_GameMsgDownHead
{
	//CT_DWORD    dwRedPacketIndex;                       //红包期数
	CT_WORD     wGameID;								//游戏ID[当收到这条消息，玩家不在对应房间，不播放红包]
	CT_WORD     wKindID;								//类型ID
	CT_WORD     wRoomKindID;							//房间类型ID
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif //CMD_PLAZA_HEAD_FILE



