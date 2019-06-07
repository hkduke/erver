
/*----------------- timeFunction.cpp
*
* Copyright (C): 2017  GaoXin Network
* Author       : luwei
* Version      : V1.0
* Date         : 2017-09-22 17:23:16
*--------------------------------------------------------------
*
*------------------------------------------------------------*/

#include "ipFunction.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

//-------------------------------------------------------------
//------------------------------ 通过域名获取IP
bool GetIpByDomainName(char *szHost, std::vector<std::string>&  vecIp)
{
	//初始化环境
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);

	struct addrinfo hints;
	struct addrinfo *res, *cur;
	struct sockaddr_in *addr;
	// 保存Ip地址信息 xxx.xxx.xxx.xxx
	char m_IpAddr[16];

	//初始化 hints
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;	//IPv4
	hints.ai_flags = AI_PASSIVE; //匹配所有 IP 地址
	hints.ai_protocol = 0;        //匹配所有协议
	hints.ai_socktype = SOCK_STREAM; //流类型

									 //获取 ip address ， res 指向一个链表Address Information链表
	int ret = getaddrinfo(szHost, NULL, &hints, &res);
	if (ret == -1)
	{
		//perror("getaddrinfo");
		//exit(-1);

		return false;
	}

	//输出获取的信息
	for (cur = res; cur != NULL; cur = cur->ai_next)
	{
		addr = (struct sockaddr_in *) cur->ai_addr; //获取当前 address

		sprintf(m_IpAddr, "%d.%d.%d.%d", addr->sin_addr.S_un.S_un_b.s_b1,
			addr->sin_addr.S_un.S_un_b.s_b2,
			addr->sin_addr.S_un.S_un_b.s_b3,
			addr->sin_addr.S_un.S_un_b.s_b4);

		//printf("%s\n", m_IpAddr); //输出到控制台
		vecIp.push_back(m_IpAddr);
	}
	//清除环境信息
	WSACleanup();
	return true;
}
#else //WIN32
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
bool GetIpByDomainName(char *szHost, std::vector<std::string>& vecIp)
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET 或 AF_INET6 可以指定版本
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(szHost, NULL, &hints, &res)) != 0)
	{
		//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return false;
	}

	for (p = res; p != NULL; p = p->ai_next)
	{
		void *addr;
		//char *ipver;

		// 取得本身位址的指標，
		// 在 IPv4 與 IPv6 中的欄位不同：
		if (p->ai_family == AF_INET)
		{ // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			//ipver = "IPv4";
		}
		else 
		{ // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			//ipver = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		vecIp.push_back(std::string(ipstr));
		//printf(" %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(res); // 釋放鏈結串列
	return true;
}
#endif // WIN32

