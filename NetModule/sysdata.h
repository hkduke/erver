#ifndef CCSYSDATA_H_
#define CCSYSDATA_H_

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif



#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#define			 SET_DES						0x00000001               //des加密标志                                 
#define			 SET_CRC						0x00000002               //crc验证标志
#define          SET_CPS                        0x00000004               //压缩标志

#define			 SYS_NET_MAX_SLEEP_TIME         (2)                     //单位毫秒

//初始化消息块数
#define			SYS_EXPANT_NEW_OBJ_NUM			(1024)					//扩展对象个数
#define			SYS_INIT_MSG_BLOCK_NUM			(10000)					//初始化消息块数10000块


//命令子消息类型  最多32个
//网络消息   
#define			MSC_S_SOCKET_LINK				0x00000001				 //连接打开
#define			MSC_S_SOCKET_SHUT				0x00000002				 //连接关闭
#define			MSC_S_NETWORK_BIND				0x00000004				 //网络应答
#define			MSC_S_NETWORK_SHUT				0x00000008				 //网络关闭
#define			MSC_S_HANDLE_INFO				0x00000016               //连接/网络信息(NET_BIT)
#define			MSC_S_HANDLE_TIMER				0x00000032				 //网络定时器

#define MSG_HEART_BEAT_MAIN	0
#define SUB_HEART_BEAT		1

struct CMD_KN_DetectSocket
{
	CT_DWORD								dwSendTickCount;					//发送时间
	//CT_DWORD								dwRecvTickCount;					//接收时间
};

#ifndef _DEBUG
#define HEART_BEAT_TIMEOUT	50000
#else
#define HEART_BEAT_TIMEOUT  500000
#endif

	//网络错误码
#define			SOCKET_ERROR					(-1)                     //网络错误标志

#ifdef SOCKETS_NAMESPACE
}
#endif


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif









