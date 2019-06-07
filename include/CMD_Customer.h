#ifndef CMD_CUSTMOER_H_
#define CMD_CUSTMOER_H_
#include "CTType.h"
#include "CTData.h"
#include "CGlobalData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//web服务器和dip服务服通讯的消息定义Customer 
#define MSG_CUSTOMER_MAIN					100

//客服端协议
//c->s
#define SUB_CUSTOMER2SERVER_CHAT			    1002			//客服发聊天[MSG_C2S_Chat_To_User]

//s->c
#define SUB_SERVER2CUSTOMER_LOGIN			    1010			//客服登录返回[MSG_S2C_Cusotmer_Login]
#define SUB_SERVER2CUSTOMER_CHAT			    1011			//客服接收聊天[MSG_S2C_Chat_To_Customer]
#define SUB_SERVER2CUSTOMER_USERLIST		    1012			//用户列表[MSG_S2C_User_List]
#define SUB_SERVER2CUSTOMER_DDZ_WAIT_USER	    1013			//斗地主等待人数[MSG_S2C_DDZ_WaitingUserCount]

//用户端协议
//c->s
#define SUB_CLIENT2SERVER_USER_VISIT		    1030			//玩家访问[MSG_C2S_User_Visit]
#define SUB_CLIENT2SERVER_CHAT				    1031			//玩家发聊天[MSG_C2S_Chat_To_Customer]
//#define SUB_CLIENT2SERVER_USER_VISIT_REPORT     1032			//玩家访问举报[MSG_C2S_User_Visit]
//#define SUB_CLINET2SERVER_REPORT			    1033			//玩家举报[MSG_C2S_Chat_To_Customer]
#define SUB_CLINET2SERVER_READ_MSG			    1034			//玩家阅读消息返回[MSG_C2S_User_Read_CustomerMsg]

//s->c
#define SUB_SERVER2CLIENT_CHAT				    1040			//玩家接收聊天[一个byte+MSG_S2C_Chat_To_User]
//#define SUB_SERVER2CLIENT_CHAT_REPORT		    1041			//玩家接收举报聊天[MSG_S2C_Chat_To_User]
#define SUB_SERVER2CLIENT_CHAT_RECORD		    1042			//玩家接收聊天记录[一个byte+10条MSG_S2C_ChatRecord_To_User]
//#define SUB_SERVER2CLIENT_CHAT_RECORD_REPORT    1043            //玩家接收举报聊天记录[10条MSG_S2C_ChatRecord_To_User]


struct MSG_C2S_Customer_Login
{
	char	szCustomerName[32];				//客服名称
	char	szPassword[33];					//客服密码
};

struct MSG_C2S_Chat_To_User
{
	CT_DWORD dwUserID;						//玩家ID
	char	szContent[300];					//聊天内容
};

struct MSG_C2S_User_Leave
{
	CT_DWORD  dwUserID;						//玩家ID
};

struct MSG_S2C_Cusotmer_Login
{
	int		nResult;						//登录结果(0:登录成功，1:该客服已经登录)
};

struct MSG_S2C_Chat_To_Customer
{
	CT_DWORD dwUserID;						//玩家ID
	char	szNickName[32];					//玩家昵称
	char	szContent[300];					//聊天内容
	char	szTime[20];						//时间
};

struct MSG_S2C_DDZ_WaitingUserCount
{
	CT_WORD	 wKindID;						//游戏类型（1是小资场 2是老板场 3土豪场）
	CT_WORD	 wWaitingUserCount;				//等待人数
};

struct MSG_S2C_User_List
{
	CT_DWORD dwUserID;						//玩家ID
	char	szNickName[32];					//玩家昵称
};

struct MSG_C2S_User_Visit
{
	CT_DWORD 	dwUserID;						//玩家ID
	CT_BYTE		cbMsgType;						//聊天类型（1:普通聊天 2:举报聊天(暂弃用) 3:充值聊天 4:兑换聊天）
	//char    szNickName[32];					//玩家昵称
};

struct MSG_C2S_Chat_To_Customer
{
	CT_DWORD dwUserID;						//用户ID
	CT_BYTE  cbType;						//聊天类型（1:普通聊天 2:举报聊天(暂弃用) 4:充值聊天 5:兑换聊天）
	char	 szContent[300];				//聊天内容
};

struct MSG_C2S_Chat_To_CustomerEx
{
	CT_DWORD dwUserID;						//用户ID
	CT_BYTE  cbType;						//聊天类型（1:普通聊天 2:举报聊天(暂弃用) 4:充值聊天 5:兑换聊天）
	char	 szContent[300];				//聊天内容
	char     szClientIp[IP_LEN];			//客户端IP
};

struct MSG_C2S_User_Read_CustomerMsg
{
    CT_DWORD    dwUserID;                   //用户ID
	CT_DWORD	dwMsgID;					//消息ID
};

/*struct MSG_S2C_User_Visit
{
	int		nResult;						//结果(0:访问成功， 1:没有在线客服)
	char	szBindCustomerName[32];			//绑定的客服名称
};*/

struct MSG_S2C_Chat_To_User
{
    CT_DWORD    dwMsgId;                    //消息ID
    CT_BYTE 	cbContentType;				//消息内容类型(1:普通消息 2:图片链接)
	char	    szContent[300];				//聊天内容
	char	    szTime[20];					//时间
};

struct MSG_S2C_ChatRecord_To_User
{
	CT_DWORD	dwMsgId;					//消息ID
	CT_BYTE		cbType;					    //消息类型：1、玩家发送的信息 2、客服发送的信息
	CT_BYTE 	cbContentType;				//消息内容类型 (1:普通消息 2:图片链接)
	CT_BYTE     cbIsRead;                   //是否已读: 1、
	char 		szContent[300];				//消息内容
	char 		szTime[20];					//消息时间
};

//////////////////////////////////////////////////////////////////////////////////
//中心服务器和客服服务服通讯的消息定义
#define MSG_CUSTOMERCS_MAIN						101

//C->S
#define SUB_CUSTOMER2CS_REGSTER					10100		//注册

struct CMD_CustomerServer_Info
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_CHAR		szServerIP[STRING_IP_LEN];			//IP地址
	CT_CHAR		szServerPubIP[STRING_IP_LEN];		//公网IP
	CT_CHAR		szServerName[SERVER_NAME_LEN];		//服务名称
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和客服服务服通讯的消息定义
#define MSG_CUSTOMERPS_MAIN						102

//代理服到客服注册
#define SUB_PS2CUSTOMER_BIND					10200		//绑定

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif