/* ----------------------------------------------------------------------------------------------------------

 ----------------------------------------------------------------------------------------------------------*/

#ifndef __Log_h__
#define __Log_h__


#include <cstdarg>


typedef enum
{
	EVT_BEGIN,
	EVT_USER_LOGIN,
	EVT_USER_LOGOUT,
	EVT_NEW_USER,
	EVT_PROFILE_CHANGE,
	EVT_USER_BAN,
	EVT_START_SINGLEPLAYER_SESSION,
	EVT_END_SINGLEPLAYER_SESSION,
	EVT_START_SINGLEPLAYER_ROUND,
	EVT_END_SINGLEPLAYER_ROUND,
	EVT_LEVEL_ADVANCE,
	EVT_SAVE_GAME,
	EVT_LOAD_GAME,
	EVT_START_MULTIPLAYER_ROUND,
	EVT_END_MULTIPLAYER_ROUND,
	EVT_ENTER_CHATROOM,
	EVT_LEAVE_CHATROOM,
	EVT_SEND_CHAT_MESSAGE,
	EVT_SEND_MAIL_MESSAGE,
	EVT_SEND_PRIVATE_CHATMESSAGE,
	EVT_SEND_MEGAPHONE_MESSAGE,
	EVT_FRIEND_ACTION,
	EVT_REPORT_ABUSE,
	EVT_MAIL_ACTION,
	EVT_FILTER_PROFANITY,
	EVT_INVENTORY_CHANGE,
	EVT_USER_DISCONNECT,
	EVT_SINGLEPLAYER_LEVELUP,
	EVT_SINGLEPLAYER_GAMEOVER,
	EVT_INSTALLGAME_LOG,
	EVT_BAN_PLAYER,
	EVT_EXCHANGE_MONEY,
	EVT_ENTER_MULTIPLAYER_GAME,
	EVT_END_MULTIPLAYER_GAME,
	EVT_ENTER_MULTIPLAYER_MATCH,
	EVT_END_MULTIPLAYER_MATCH,
	EVT_MULTIPLAYER_RECONNECT,
	EVT_MULTIPLAYER_DISCONNECT,
	EVT_MULTIPLAYER_LOSELIFE,
	EVT_MULTIPLAYER_CHEAT,
	EVT_MULTIPLAYER_PROPSUSAGE,
	// add other event here.
	EVT_END
} eEventType;

typedef enum
{
	LogLevel_Normal,
	LogLevel_Warning,
	LogLevel_Error,
	LogLevel_Failed,
	LogLevel_Debug,
	LogLevel_Temp,
} eLogLevel;

//------------------------------------------------------------
#if defined(_LINUX)
#	include <sys/syslog.h>

	inline void Log_Init(const char * iden, int flag = LOG_PID|LOG_CONS|LOG_PERROR)
	{
		openlog(iden, flag, LOG_USER);
	}

	// event log
	void App_Log(eEventType etype, const char * szFormat, ...);

	// for handle backtrace
	void TraceHandler(int signum);

#else	// win32 platform

	// fake for syslog
	typedef enum { LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG };

	// init log
	void Log_Init(const char * iden, int = 0);

	// fake for syslog
	void syslog(int, const char*, ...);
	void vsyslog(int, const char*, va_list);

	// event log
	void App_Log( eEventType etype, const char * szFormat, ...);

#endif	// end win32 platform

//------------------------------------------------------------

template <int PRIORITY>	struct MakeRule { MakeRule() {} };

#define LOG_NULL	(-1)
#define MAPLOG(name, priority)	const MakeRule<priority> name;

template <typename Rule>
inline void Log_Msg(const Rule& rule, const char * szFormat, ...);


template<> 
inline void Log_Msg< MakeRule<LOG_DEBUG> >(const MakeRule<LOG_DEBUG>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_DEBUG, szFormat, ap);					
	va_end(ap);															
}

template<> 
inline void Log_Msg< MakeRule<LOG_ALERT> >(const MakeRule<LOG_ALERT>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_ALERT, szFormat, ap);					
	va_end(ap);															
}


template<> 
inline void Log_Msg< MakeRule<LOG_CRIT> >(const MakeRule<LOG_CRIT>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_CRIT, szFormat, ap);					
	va_end(ap);															
}


template<> 
inline void Log_Msg< MakeRule<LOG_EMERG> >(const MakeRule<LOG_EMERG>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_EMERG, szFormat, ap);					
	va_end(ap);						
}

template<> 
inline void Log_Msg< MakeRule<LOG_INFO> >(const MakeRule<LOG_INFO>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_INFO, szFormat, ap);					
	va_end(ap);
}

template<> 
inline void Log_Msg< MakeRule<LOG_NOTICE> >(const MakeRule<LOG_NOTICE>&, const char * szFormat, ...) 
{
	va_list ap;															
	va_start(ap, szFormat);												
	vsyslog(LOG_NOTICE, szFormat, ap);					
	va_end(ap);
}

template<> 
inline void Log_Msg< MakeRule<LOG_NULL> >(const MakeRule<LOG_NULL>&, const char *, ...) 
{
	// empty body.
}

MAPLOG(Log_Normal,	LOG_DEBUG);
MAPLOG(Log_Warning, LOG_ALERT);
MAPLOG(Log_Error,	LOG_CRIT);
MAPLOG(Log_Failed,	LOG_EMERG);
MAPLOG(Log_Debug,	LOG_DEBUG);
MAPLOG(Log_Temp,	LOG_NULL);

enum AppEnum
{
	AE_Login_Success = 1,
	AE_Login_Timeout,
	AE_Login_Bad_CK,
	AE_Login_Banned,
	AE_Login_Repeated,
	AE_Login_Error,
	AE_Login_Exception,

	AE_Logout_User_logout,
	AE_Logout_Inactivity,
	AE_Logout_Lost_connection,
	AE_Logout_Kicked,

	AE_Male,
	AE_Female,

	AE_Newuser_Success,
	AE_Newuser_Exist,
	AE_Newuser_Database_error,
	AE_Newuser_Nickname_denied,
	AE_Newuser_Exception,

	AE_Profile_BasicInfo,
	AE_Profile_Caption,
	AE_Profile_Icon,
	AE_Profile_Skin,
	AE_Profile_Token,

	AE_Profile_Result_Success,
	AE_Profile_Result_Database_error,
	AE_Profile_Result_Exception,

	AE_Game_Session_Result_User,
	AE_Game_Session_Result_administrator,
	AE_Game_Session_Result_Time_expired,
	
	AE_Game_Save_Result_Success,
	AE_Game_Save_Result_File_Success,
	AE_Game_Save_Result_Fs_error,
	AE_Game_Save_Result_Db_error,
	AE_Game_Save_Result_Exception,
	
	AE_Game_Load_Result_Success,
	AE_Game_Load_Result_No_record,
	AE_Game_Load_Result_Exception,
	AE_Game_Load_Result_File_Success,
	AE_Game_Load_Result_File_No_record,
	AE_Game_Load_Result_Fs_error,
	
	AE_Enter_Chat_Result_Success,
	AE_Enter_Chat_Result_Room_full,
	AE_Enter_Chat_Result_Banned,
	AE_Enter_Chat_Result_Exception,
	AE_Enter_Chat_Result_Room_iderror,
	AE_Enter_Chat_Result_Repeat_login,
	AE_Enter_Chat_Result_Room_closed,
	
	AE_Leave_Chat_Kicked_off,
	AE_Leave_Chat_User_leaving,
	AE_Leave_Chat_Connect_lost,
	AE_Leave_Chat_Exception,

	AE_Send_Chat_Message_Success,
	AE_Send_Chat_Message_No_permission,
	AE_Send_chat_message_Exception,
	
	AE_Send_Mail_Result_Success,
	AE_Send_Mail_Result_Blocked,
	AE_Send_Mail_Result_Mailbox_full,
	AE_Send_Mail_Result_Exception,
	
	AE_Friend_Action_Add_friend,
	AE_Friend_Action_Confirm_adding,
	AE_Friend_Action_Decline_adding,
	AE_Friend_Action_Remove_friend,

	AE_Mail_Action_Read,
	AE_Mail_Action_Delete,

	AE_Mail_Result_Success,
	AE_Mail_Result_Exception,

	AE_Inventory_Action_bought,
	AE_Inventory_Action_Spent,
	AE_Inventory_Action_Add_automatic,
	AE_Inventory_Action_Add_manual,

	AE_Inventory_Result_Success,
	AE_Inventory_Result_No_money,
	AE_Inventory_Result_Exception,

	AE_ExchangeMoney_Result_Success,
	AE_ExchangeMoney_Result_Exception,

	AE_Reconnection_Result_Success,
	AE_Reconnection_Result_Fail,
};

enum GameNameEnum{
	GAMEID_PLATFORM				= 0,
	GAMEID_PEGGLE              = 100,
	GAMEID_ZUMA                = 200,
	GAMEID_FEEDINGFRENZY2      = 300,
	GAMEID_BEJEWELED2          = 400,
	GAMEID_DYNOMITE            = 500,
	GAMEID_CHUZZLE             = 600,
	GAMEID_BIGMONEY            = 700,
	GAMEID_INSANIQUARIUM       = 800,
	GAMEID_BEJEWELEDTWIST      = 900,
	GAMEID_PVZ                 = 1000,
	GAMEID_AA2                 = 1100,
	GAMEID_MRPI2               = 1200,
	GAMEID_ZUMA2               = 1300,
	GAMEID_ZUMA2_DUEL		   = 1310,
	GAMEID_SUPERZUMA		   = 1311,
	GAMEID_BEJEWELED3          = 1400,
};

#define Client_Succeed					0							/* client connect succeed */
#define Err_ClientClose					-1							/* client connect close */ 
#define Err_ClientTimeout				-2							/* client timeout */
#define Err_PacketError					-3							/* client send data error */ 
#define Err_TCPBuffOver					-4							/* TCP buffer full */ 
#define Err_SendToLogic					-5							/* put msg to msgqueue failed */
#define Err_System						-6							/* system error */
#define Err_ServerKicked				-7
#define Err_PlayerClose					-8
#define Err_ServerClose					-9
#define Err_ExitChatRoomClose			-10
#define Err_CKValidFailedClose			-11
#define Err_UnknownClose				-12

inline const char* GetLostConnectErrorStr( short sError )
{
#ifdef MAP_EVENT_TO_SEVERITY
#undef MAP_EVENT_TO_SEVERITY
#endif
#define MAP_EVENT_TO_SEVERITY(a, b) case a: return b;

	switch ( sError )
	{
		MAP_EVENT_TO_SEVERITY( Client_Succeed,				"Client_Success" );
		MAP_EVENT_TO_SEVERITY( Err_ClientClose,				"Err_ClientClose" );
		MAP_EVENT_TO_SEVERITY( Err_ClientTimeout,			"Err_ClientTimeout" );
		MAP_EVENT_TO_SEVERITY( Err_PacketError,				"Err_PacketError" );
		MAP_EVENT_TO_SEVERITY( Err_TCPBuffOver,				"Err_TCPBuffOver" );
		MAP_EVENT_TO_SEVERITY( Err_SendToLogic,				"Err_SendToLogic" );
		MAP_EVENT_TO_SEVERITY( Err_System,					"Err_System" );
		MAP_EVENT_TO_SEVERITY( Err_ServerKicked,			"Err_ServerKicked" );
		MAP_EVENT_TO_SEVERITY( Err_PlayerClose,				"Err_PlayerClose" );
		MAP_EVENT_TO_SEVERITY( Err_ServerClose,				"Err_ServerClose" );
		MAP_EVENT_TO_SEVERITY( Err_ExitChatRoomClose,		"Err_ExitChatRoomClose" );
		MAP_EVENT_TO_SEVERITY( Err_CKValidFailedClose,		"Err_CKValidFailedClose" );
		MAP_EVENT_TO_SEVERITY( Err_UnknownClose,			"Err_UnknownClose" );



		default: return "None";
	}
	return "None";
}

inline const char* AppGetLogString( int nEnum )
{
#ifdef MAP_EVENT_TO_SEVERITY
#undef MAP_EVENT_TO_SEVERITY
#endif
#define MAP_EVENT_TO_SEVERITY(a, b) case a: return b;
	switch ( nEnum )
	{
		MAP_EVENT_TO_SEVERITY( AE_Login_Success,					"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Timeout,					"Timeout" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Bad_CK,						"Bad_CK" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Repeated,					"Repeated" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Banned,						"Banned" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Error,						"Error" );
		MAP_EVENT_TO_SEVERITY( AE_Login_Exception,					"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Logout_User_logout,				"User_logout" );
		MAP_EVENT_TO_SEVERITY( AE_Logout_Inactivity,				"Inactivity" );
		MAP_EVENT_TO_SEVERITY( AE_Logout_Lost_connection,			"Lost_connection" );
		MAP_EVENT_TO_SEVERITY( AE_Logout_Kicked,					"Kicked" );

		MAP_EVENT_TO_SEVERITY( AE_Male,								"Male" );
		MAP_EVENT_TO_SEVERITY( AE_Female,							"Female" );

		MAP_EVENT_TO_SEVERITY( AE_Newuser_Success,					"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Newuser_Exist,					"Exist" );
		MAP_EVENT_TO_SEVERITY( AE_Newuser_Database_error,			"Database_error" );
		MAP_EVENT_TO_SEVERITY( AE_Newuser_Nickname_denied,			"Nickname_denied" );
		MAP_EVENT_TO_SEVERITY( AE_Newuser_Exception,				"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Profile_BasicInfo,				"BasicInfo" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Caption,					"Caption" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Icon,						"Icon" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Skin,						"Skin" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Token,					"token" );

		MAP_EVENT_TO_SEVERITY( AE_Profile_Result_Success,			"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Result_Database_error,	"Database_error" );
		MAP_EVENT_TO_SEVERITY( AE_Profile_Result_Exception,			"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Game_Session_Result_User,			"User" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Session_Result_administrator,"System_administrator" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Session_Result_Time_expired,	"Time_expired" );

		MAP_EVENT_TO_SEVERITY( AE_Game_Save_Result_Success,			"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Save_Result_File_Success,	"Fs_success" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Save_Result_Fs_error,		"Fs_error" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Save_Result_Db_error,		"Db_error" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Save_Result_Exception,		"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_Success,			"Sucess" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_No_record,		"No_record" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_Exception,		"Excepption" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_File_Success,	"Fs_success" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_File_No_record,	"Fs_no_record" );
		MAP_EVENT_TO_SEVERITY( AE_Game_Load_Result_Fs_error,		"Fs_error" );

		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Success,		"Sucess" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Room_full,		"Room_full" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Banned,			"Banned" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Exception,		"Exception" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Room_iderror,	"Room_iderror" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Repeat_login,	"Repeat_login" );
		MAP_EVENT_TO_SEVERITY( AE_Enter_Chat_Result_Room_closed,	"Room_closed" );

		MAP_EVENT_TO_SEVERITY( AE_Leave_Chat_Kicked_off,			"Kicked_off" );
		MAP_EVENT_TO_SEVERITY( AE_Leave_Chat_User_leaving,			"User_leaving" );
		MAP_EVENT_TO_SEVERITY( AE_Leave_Chat_Connect_lost,			"Connect_lost" );
		MAP_EVENT_TO_SEVERITY( AE_Leave_Chat_Exception,				"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Send_Chat_Message_Success,		"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Send_Chat_Message_No_permission,	"No_permission" );
		MAP_EVENT_TO_SEVERITY( AE_Send_chat_message_Exception,		"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Send_Mail_Result_Success,			"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Send_Mail_Result_Blocked,			"Blocked" );
		MAP_EVENT_TO_SEVERITY( AE_Send_Mail_Result_Mailbox_full,	"Mailbox_full" );
		MAP_EVENT_TO_SEVERITY( AE_Send_Mail_Result_Exception,		"Exception" );
		
		MAP_EVENT_TO_SEVERITY( AE_Friend_Action_Add_friend,			"Add_friend" );
		MAP_EVENT_TO_SEVERITY( AE_Friend_Action_Confirm_adding,		"Confirm_adding" );
		MAP_EVENT_TO_SEVERITY( AE_Friend_Action_Decline_adding,		"Decline_adding" );
		MAP_EVENT_TO_SEVERITY( AE_Friend_Action_Remove_friend,		"Remove_friend" );

		MAP_EVENT_TO_SEVERITY( AE_Mail_Action_Read,					"Read" );
		MAP_EVENT_TO_SEVERITY( AE_Mail_Action_Delete,				"Delete" );

		MAP_EVENT_TO_SEVERITY( AE_Mail_Result_Success,				"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Mail_Result_Exception,			"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Inventory_Action_bought,			"Bought" );
		MAP_EVENT_TO_SEVERITY( AE_Inventory_Action_Spent,			"Spent" );
		MAP_EVENT_TO_SEVERITY( AE_Inventory_Action_Add_automatic,	"Add_automatic" );
		MAP_EVENT_TO_SEVERITY( AE_Inventory_Action_Add_manual,		"Add_manual" );

		MAP_EVENT_TO_SEVERITY( AE_Inventory_Result_Success,			"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Inventory_Result_No_money,		"No_money" );
		MAP_EVENT_TO_SEVERITY( AE_Inventory_Result_Exception,		"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_ExchangeMoney_Result_Success,		"Success" );
		MAP_EVENT_TO_SEVERITY( AE_ExchangeMoney_Result_Exception,	"Exception" );

		MAP_EVENT_TO_SEVERITY( AE_Reconnection_Result_Success,		"Success" );
		MAP_EVENT_TO_SEVERITY( AE_Reconnection_Result_Fail,			"Exception" );

	default: return "None";
	}

	return "None";
#undef MAP_EVENT_TO_SEVERITY
}

#endif

