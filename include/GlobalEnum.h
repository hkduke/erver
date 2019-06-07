#ifndef _GLOBAL_ENUM_H_
#define _GLOBAL_ENUM_H_

//服务器类型
enum ServerType
{
	CENTER_SERVER,
	DB_SERVER,
	PROXY_SERVER,
	LOGIN_SERVER,
	GAME_SERVER,
	DIP_SERVER,
	CUSTOMER_SERVER,
	UNKNOW_SERVER,
};

//服务器状态
enum ServerState
{
	SERVER_STOP = 0,	//维服状态
	SERVER_RUN	= 1,	//运行状态
};

//内部网络链接类型
enum ConnectType
{
	CONNECT_CENTER,		//中心服
	CONNECT_LOGIN,		//登录服
	CONNECT_GAME,		//游戏服
	CONNECT_DB,			//DB服
	CONNECT_CUSTOMER,	//客服服
	CONNECT_GAME_THIRD,	//第三方游戏服务器
	CONNECT_MAX,
};

//设备类型
enum enMachineType
{
	MACHINE_TYPE_IOS = 1 ,		//iphone
	MACHINE_TYPE_ANDROID = 2,	//android
};

//游戏枚举
enum enGameID
{
	GAME_PHZ = 1, 			//跑胡子
	GAME_PDK = 2,			//跑得快
	GAME_NN = 3,			//牛牛
	GAME_MJ = 4,			//麻将
	GAME_DDZ = 5,			//斗地主
	GAME_ZJH = 6,			//炸金花
	GAME_YYDB = 7,          //一元夺宝
	GAME_FXGZ = 8,			//福星高照(水果机)
	GAME_HBSL = 9,			//红包扫雷
	GAME_JZNC = 10,         //金猪纳财

	GAME_BR = 100,			//百人游戏
	GAME_FISH = 168,		//金蝉捕鱼
};

//游戏玩法类型
enum enPHZGameKindID
{
	GAME_PHZ_LDFPF = 1,		//跑胡子娄底放炮法
	GAME_PHZ_SYBP  = 2,		//邵阳剥皮
};

//游戏玩法类型
enum enPDKGameKindID
{
	GAME_PDK_16 = 1,		//跑得快16张
};

//游戏玩法类型
enum enDDZGameKindID
{
	GAME_DDZ_1 = 1,			//普通斗地主
};

//游戏玩法类型
enum enNNGameKindID
{
	GAME_QZ_NN = 1,			//抢庄牛牛
};

//游戏玩法类型
enum enMJGameKindID
{
	GAME_MJ_HZ = 1,				//红中麻将
	GAME_MJ_ZZ = 2,				//转转麻将
	GAME_MJ_ER = 3,             //二人麻将
	GAME_MJ_XZ = 4,             //血战麻将
};

//游戏玩法类型
enum emZJHGameKindID
{
	GAME_ZJH_1 = 1,				//炸金花
};

//游戏玩法类型
enum emBRGameKindID
{
	GAME_BR_NN  = 1,			//百人牛牛
	GAME_BR_TDZ = 2,			//百人推对子
	GAME_BR_HHDZ = 3,			//红黑大战
	GAME_BR_LHD = 4,			//龙虎斗
};

//游戏玩法类型
enum emFishGameKindID
{
	GAME_FISH_JC = 1,			//金蝉捕鱼
	GAME_FISH_SDS = 2,			//黄金圣斗士
	GAME_FISH_SHCS = 3,         //深海传说
	GAME_FISH_JS = 4,           //极速捕鱼
};

//游戏房间类型
enum enGameRoomKind
{
	PRIVATE_ROOM = 0,			//房卡模式
	PRIMARY_ROOM,				//初级房
	MIDDLE_ROOM,				//中级房
	SENIOR_ROOM,				//高级房
    FREE_ROOM,                  //免费场
	MATCH_ROOM,					//比赛房
};

//群组房间类型
enum enGroupRoomKind
{
	GX_GROUP_ROOM = 0,			//搞两把群组房间
	WX_GROUP_ROOM = 1,			//微信群组房间
};

enum enPRoomState
{
	PROOM_START = 1,		//房间开始
	PROOM_END,				//房间结束
};

//游戏模式
enum enGameMode
{
	PRIMARY_MODE = 1,		//私人场
	SCORE_MODE = 2,			//金币场
};

//玩家帐号状态
enum enAccountStatus
{
	en_Account_Normal = 1,	//帐号正常
	en_Account_SEAL	 = 2,	//帐号被封
};

//玩家性别
enum enSex
{
	en_Boy = 1,				//男
	en_Girl = 2,			//女
};

//玩家头像
enum  enHeadId
{
	en_HeadId_Boy_Start = 1,			//男头像开始索引
	en_HeadId_Girl_Start = 51,			//女头像开始索引

	en_HeadId_Count = 6,			//头像个数(男女各6个)
};

enum enRegisterErrorCode
{
	REGISTER_SUCC				= 0,			//注册成功
};

enum enLoginErrorCode
{
	LOGIN_ACCOUNTS_NOT_EXIST	= 1,		//帐号不存在
	LOGIN_PASSWORD_ERCOR		= 2,		//密码错误
	LOGIN_REGISTER_FAIL			= 3,		//注册失败
	LOGIN_SEAL_ACCOUNTS			= 4,		//封号错误
	LOGIN_LONG_NULLITY			= 5,		//禁止登录
	LOGIN_NETBREAK				= 6,		//网络中断
	LOGIN_NOFIND_GAMEITEM		= 7,		//没有找到此类游戏
	LOGIN_IN_OTHER_GAME			= 8,		//在其他游戏中
	LOGIN_VERSION_TOO_LOW		= 9,		//版本太低

	REGISTER_CHAR_EMPTY			= 10,		//帐号或密码为空
	REGISTER_ILLEGAL_CHAR		= 11,		//非法字符
	REGISTER_ACCOUNT_REPEAT		= 12,		//帐号重复
	REGISTER_VERIFYCODE_ERROR	= 13,		//验证码错误
	REGISTER_IP_LIMITED			= 14,		//您的ip注册已达上限，请更换ip或者明日再试

	REGISTER_UNKNOWN			= 20,		//未知错误
};

//创建房间错误码
enum enCreatePRoomErrorCode
{
	CREATE_PROOM_ERROR_SESSION = 1,				//session错误(没有找到玩家的连接信息)
	CREATE_PROOM_GAMESERVER_NOENOUGH = 2,		//没有找到空闲的服务器
	CREATE_PROOM_GEM_NOENOUGH = 3,				//钻石不够
	CREATE_PROOM_ERROR_PARAM = 4,				//参数错误(如创建5人房，但实际服务器没有5人房)
	CREATE_PROOM_HAS_PROOM = 5,					//已经有私人房间
};

//进入房间错误码
enum enEnterPRoomErrorCode
{
	ENTER_PROOM_ERROR_SESSION = 1,				//session错误(没有找到玩家的连接信息)
	ENTER_PROOM_NOTFIND = 2,					//没有这个房间号
	ENTER_PROOM_GAMENOEXIST = 3,				//没有找到私人房间所在的服务器
	ENTER_PROOM_TABLE_NOENOUGH = 4,				//私人房间的服务器的桌子不够
	ENTER_PROOM_SEAT_FULL = 5,					//私人房间的已满员
	ENTER_PROOM_USERNOEXIST = 6,				//玩家不存在或者没有找玩家信息
	ENTER_PROOM_HAS_OTHER_PROOM = 7,			//已有其他私人房间，不能进入此房间
	ENTER_PROOM_VIP_OWNER_OFFLINE = 8,			//申请进入VIP私人房间失败，房主不在线
	ENTER_PROOM_VIP_FULL_APPLY = 9,				//申请进入VIP私人房间失败，申请已达到最大人数
	ENTER_PROOM_VIP_ALREADY_APPLY = 10,			//已经申请，不需要重新申请，等待房主确认
	ENTER_PROOM_NOT_THIS_GAME = 11,				//当前房间不属于这个游戏
	ENTER_PROOM_GAME_START_ALREADY = 12,		//游戏已经开始
	ENTER_PROOM_NOT_IN_GROUP = 13,				//不是群成员
};

//进入普通房间错误码
enum enEnterRoomErrorCode
{
	ENTER_ROOM_ERROR_SESSION 		= 1,		//session错误(没有找到玩家的连接信息)
	ENTER_ROOM_GAMENOEXIST 			= 2,		//没有找到桌子所在的服务器
	ENTER_ROOM_TABLE_NOENOUGH 		= 3,		//服务器的桌子不够
	ENTER_ROOM_SEAT_FULL 			= 4,		//桌子已满员
	ENTER_ROOM_USERNOEXIST 			= 5,		//玩家不存在或者没有找玩家信息
	ENTER_ROOM_SCORE_NOT_ENOUGH 	= 6,		//金币不足
	ENTER_ROOM_GAME_START_ALREADY 	= 7,		//游戏已经开始
	ENTER_ROOM_SCORE_LIMIT 			= 8,		//拥有金币与进入场次需求不符
	ENTER_ROOM_USER_IN_GAME			= 9,		//用户已经在游戏中(配桌时)
	ENTER_ROOM_SERVER_STOP 			= 10,		//服务器处于维服状态，被迫下线
	ENTER_ROOM_LONG_TIME_NOT_OP 	= 11,		//长时间没有操作被踢房间
	ENTER_ROOM_WINSCORE_LIMITE 		= 12,		//输赢分达到上下限制(抢庄牛牛托管用)
	ENTER_ROOM_GAME_OVER			= 13,       //游戏已经结束
};

//群组相关
enum enGroupErrorCode
{
	GROUP_COUNT_MAX			            = 1,			//群组数量已经达到最大
	GROUP_NAME_ERROR		            = 2,			//群名称输入有误
	GROUP_USER_ALREADY		            = 3,			//已经是群组成员
	NOT_FOUND_GROUP			            = 4,			//没有找到这个群
	GROUP_USER_FULL			            = 5,			//群组已满员
	PERMISSION_DENIED		            = 6,			//没有权限进行此操作
	NOT_GROUP_USER			            = 7,			//非群组成员
	USER_NOT_ONLINE			            = 8,			//玩家长时间不在线，无法获取其资料
	NOT_FOUND_GAMESERVER	            = 9,			//没有找相对应的游戏服务器
	MASTER_GEM_NOT_ENOUGH	            = 10,			//群主钻石不够
	GROUP_HAS_PROOM			            = 11,			//本群还有房间未结束,不能解散
	HAS_GROUP_CANNOT_CREATE	            = 12,			//玩家拥有群，不能再创建群
    CREATE_GROUP_VIP_NOT_ENOUGH         = 13,           //vip2未达到创建群的标准
    HAS_GROUP_CANNOT_APPLY				= 14,			//玩家拥有群，不能再申请加入其他群
    UPGRADE_CONTRIBUTE_NOT_ENOUGH       = 15,           //贡献度不足，不能升级群
    UPGRADE_INCOMERATE_TOO_MUCH         = 16,           //提升收入比例不能本人的比例
    UPGRADE_GROUP_MAX                   = 17,           //群已经达到最大等级,不能再升级
};

//积分变化类型
//变化类型 兑换(1), 充值(2), 赠送(3), 充值返还(4), 签到(5), 救济金(6), 红包(7), 存入金币(8), 取出金币(9),任务奖励(10),兑换退还(11)
enum enScoreChangeType
{
	EXCHANGE_SCORE			= 1,			//兑换金币
	RECHARGE				= 2,			//充值（web端记录）
	PRESENTED_SCORE			= 3,			//赠送金币
	RECHARGE_RETURN			= 4,			//充值返还
	SIGN_IN_REWARD			= 5,			//签到奖励
	BENEFIT_REWARD			= 6,			//救济金奖励
	GET_HONGBAO				= 7,			//领取红包
	BANK_SAVE				= 8,			//银行存入
	BANK_TAKE				= 9,			//银行取出
	TASK_REWARD				= 10,			//任务奖励
	EXCHANGE_RETURN			= 11,			//兑换返还
	MAIL_REWARD				= 12,			//邮件领取
	//代理充值(13), 代理回调(14),
	REPORT_PLAYER			= 15,           //举报玩家
    CSJL_REWARD             = 16,           //财神降临奖励
    BUY_DUOBAO              = 17,           //购买一元夺宝
	YY_DUOBAO_REWARD        = 18,           //一元夺宝奖励
	YY_DUOBAO_RETURN		= 19,			//一元夺宝返还
 
	PLAY_GAME				= 20,			//游戏输赢
	HBSL_MAIL				= 21, 			//红包扫雷邮件返还

	UNKNOW_SCORE_TYPE		= 50,			//未知类型
};

enum enRechargeType
{
	RECHARGE_FAIL = 0,						//充值失败
	RECHARGE_SCORE = 1,						//充值金币
	RECHARGE_GEM = 2,						//充值钻石
	RECHARGE_BANK = 3,						//充值银行
};

//任务类型
enum enTaskType
{
	TASK_TYPE_DAILY			= 1,			//玩游戏日常任务(每天可以重复做多次)
	TASK_TYPE_WEEKLY		= 2,			//周任务
	TASK_TYPE_LOGIN			= 3,			//登录任务
};

//任务完成条件
enum enTaskCondition
{
	TASK_COND_IN_ROOM_PLAY	= 1,			//在房间玩游戏
	TASK_COND_RECHARGE		= 100,			//充值
};

//任务ID
enum enTaskID
{
	TASK_ID_IN_ANY_RANK		= 5,			//上任意排行榜
};

//充值渠道类型
enum enRechargeChannel
{
	//1微信 2支付宝 3网银 4QQ钱包 5代理
	WEIXIN_CHANNEL = 1,
	ALIPAY_CHANNEL,
	UNIONPAY_CHANNEL,
	QQPAY_CHANNEL,
	AGENCY_CHANNEL,
	MAX_CHANNEL,
};

//省份枚举
enum enProvince
{
	en_BEIJING      = 1,	//1  北京
	en_TIANJIN      = 2, 	//2  天津
	en_HEBEI        = 3, 	//3  河北
    en_SHANXI       = 4, 	//4  山西
	en_NEIMENGGU    = 5, 	//5  内蒙古
	en_LIAONING     = 6, 	//6  辽宁
	en_JILIN        = 7, 	//7  吉林
	en_HEILONGJIANG = 8, 	//8  黑龙江
	en_SHANGHAI     = 9, 	//9  上海
	en_JIANGSU      = 10,	//10  江苏
	en_ZHEJIANG     = 11,	//11  浙江
	en_ANHUI        = 12,	//12  安徽
	en_FUJIAN       = 13,	//13  福建
	en_JIANGXI      = 14,	//14  江西
	en_SHANDONG     = 15,	//15  山东
	en_HENAN        = 16,	//16  河南
	en_HUBEI        = 17,	//17  湖北
	en_HUNAN        = 18,	//18  湖南
	en_GUANGDONG    = 19,	//19  广东
	en_GUANGXI      = 20,	//20  广西
	en_HAINAN       = 21,	//21  海南
	en_CHONGQING    = 22,	//22  重庆
	en_SICHUAN      = 23,	//23  四川
	en_GUIZHOU      = 24,	//24  贵州
	en_YUNNAN       = 25,	//25  云南
	en_XIZANG       = 26,	//26  西藏
	en_SHANXI2      = 27,	//27  陕西
	en_GANSU        = 28,	//28  甘肃
	en_QINGHAI      = 29,	//29  青海
	en_NINGXIA      = 30,	//30  宁夏
    en_XINJIANG     = 31,	//31  新疆
	en_TAIWAN       = 32,	//32  台湾
	en_XIANGGANG    = 33,	//33  香港
	en_AOMEN        = 34,	//34  澳门
	en_GUOWAI       = 35,	//35  国外
    en_OTHER        = 36,	//36  其它
};


#endif

