
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
//------------------------------ ͨ��������ȡIP
bool GetIpByDomainName(char *szHost, std::vector<std::string>&  vecIp)
{
	//��ʼ������
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);

	struct addrinfo hints;
	struct addrinfo *res, *cur;
	struct sockaddr_in *addr;
	// ����Ip��ַ��Ϣ xxx.xxx.xxx.xxx
	char m_IpAddr[16];

	//��ʼ�� hints
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;	//IPv4
	hints.ai_flags = AI_PASSIVE; //ƥ������ IP ��ַ
	hints.ai_protocol = 0;        //ƥ������Э��
	hints.ai_socktype = SOCK_STREAM; //������

									 //��ȡ ip address �� res ָ��һ������Address Information����
	int ret = getaddrinfo(szHost, NULL, &hints, &res);
	if (ret == -1)
	{
		//perror("getaddrinfo");
		//exit(-1);

		return false;
	}

	//�����ȡ����Ϣ
	for (cur = res; cur != NULL; cur = cur->ai_next)
	{
		addr = (struct sockaddr_in *) cur->ai_addr; //��ȡ��ǰ address

		sprintf(m_IpAddr, "%d.%d.%d.%d", addr->sin_addr.S_un.S_un_b.s_b1,
			addr->sin_addr.S_un.S_un_b.s_b2,
			addr->sin_addr.S_un.S_un_b.s_b3,
			addr->sin_addr.S_un.S_un_b.s_b4);

		//printf("%s\n", m_IpAddr); //���������̨
		vecIp.push_back(m_IpAddr);
	}
	//���������Ϣ
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
	hints.ai_family = AF_INET; // AF_INET �� AF_INET6 ����ָ���汾
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

		// ȡ�ñ���λַ��ָ�ˣ�
		// �� IPv4 �c IPv6 �еę�λ��ͬ��
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

	freeaddrinfo(res); // ጷ�朽Y����
	return true;
}
#endif // WIN32

