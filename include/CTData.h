/******************************************************************
Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
File name:		CCData.h
Version:		1.0
Description:    用户数据
Author:			pengwen
Create Date:	2011-12-3
History:

******************************************************************/
#ifndef CCDATA_H_
#define CCDATA_H_

typedef enum
{
	STATUS_T_HDR,
	STATUS_T_DAT,
} status_t;

//消息类型  绝对不要修改
struct CMD_Command
{
	unsigned int dwMainID;			//主消息
	unsigned int dwSubID;			//子消息
	unsigned int dwDataSize;		//后面数据的长度
};

#define		SYS_NET_SENDBUF_SIZE			(50*1024)				//网络发送缓冲区大小
#define		SYS_NET_RECVBUF_SIZE			(50*1024)				//网络接收缓冲区大小
#define		SYS_NET_SENDBUF_CLIENT			(16*1024)				//发送给客户端的最大包大小（因为前端最大接收16K的包）

#define SOCKET_TCP_PACKET (SYS_NET_SENDBUF_SIZE - sizeof(CMD_Command))

#endif