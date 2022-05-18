#pragma once
#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#else
//all the linux headers here, fuck
typedef unsigned int SOCKET;
#endif

#include <string>

struct sockaddr_in initAddr_shd(UINT32 ip, int port)
{
	struct sockaddr_in target_addr;
	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = ip;
	target_addr.sin_port = htons(port);

	return target_addr;
}

struct sockaddr_in initAddr(const char* host, int port = 80)
{
	ADDRINFOA hints;
	PADDRINFOA addr = NULL;
	struct sockaddr_in result = {};
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;

	std::string hostStr = host;
	for (int i = 0; host[i]; i++)
	{
		if (host[i] == ':')
		{
			port = 0;
			hostStr = hostStr.substr(0, i);
			for (i++; host[i]; i++)
				port = port * 10 + (host[i] - '0');
			break;
		}
	}
	if (getaddrinfo(hostStr.c_str(), nullptr, &hints, &addr) == -1)
		exit(-1);

	//		InetPton(AF_INET, host, &addr_p);
	if (addr != nullptr)
	{
		result = *(struct sockaddr_in*)addr->ai_addr;
		result.sin_port = htons(port);
	}
	
	return result;
}

struct sockaddr_in initAddrW(const wchar_t* host, int port)
{
	ADDRINFOW hints;
	PADDRINFOW addr = NULL;
	struct sockaddr_in* result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	if (GetAddrInfoW(host, nullptr, &hints, &addr) == -1)
		exit(-1);

	//		InetPton(AF_INET, host, &addr_p);
	result = (struct sockaddr_in*)addr->ai_addr;
	result->sin_port = htons(port);
	return *result;
}

#ifdef _WIN32
int initWinSock()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return -1;
	return 0;
}
#endif

SOCKET initSocket()
{
	SOCKET socket_fd;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	return socket_fd;
}

int sockConn(const SOCKET* s_fd, const struct sockaddr_in* target_addr)
{
	return connect(*s_fd, (struct sockaddr*)target_addr, sizeof(*target_addr));
}

SOCKET listenSocket(const SOCKET s_fd, const int port)
{
	struct sockaddr_in srv_addr = initAddr("0.0.0.0", port);
	int s_len = sizeof(srv_addr);
	if (bind(s_fd, (struct sockaddr*)&srv_addr, s_len) < 0)
	{
		goto error;
	}

	if (listen(s_fd, 5) < 0)
	{
		goto error;
	}
	return accept(s_fd, (struct sockaddr*)&srv_addr, &s_len);
error:
	return -1;
}

void closeSocket(const SOCKET* s_fd)
{
	closesocket(*s_fd);
	WSACleanup();
}

std::string httpGetHeaderContent(std::string headerStr, const char* targetID)
{
	std::string result = "";
	size_t fIndex, tLen = strlen(targetID);
	while ((fIndex = headerStr.find(' ')) != (size_t)-1)
		headerStr = headerStr.replace(fIndex, 1, "");
	while ((fIndex = headerStr.find("\n")) != (size_t)-1)
	{
		headerStr = headerStr.substr(fIndex + 1);
		if (headerStr.substr(0, tLen) == targetID)
		{
			if ((fIndex = headerStr.find("\r")) != (size_t)-1)
				result = headerStr.substr(tLen + 1, fIndex - tLen - 1);
			else
				result = headerStr.substr(tLen + 1);
			break;
		}
	};
	return result;
}

std::string wctos(const wchar_t* source)
{
	size_t sLen = wcslen(source);
	char* c = new char[sLen * 3 + 1];
	WideCharToMultiByte(CP_UTF8, 0, source, (int)(sLen + 1), c, (int)(sLen * 3 + 1), 0, 0);

	std::string result = c;
	delete[] c;
	return result;
}

wchar_t* ctowc(const char* source)
{
	wchar_t* result;
	int sLen, rLen = MultiByteToWideChar(CP_UTF8, 0, source, sLen = (int)strlen(source), NULL, 0);
	result = new wchar_t[rLen + 1];
	MultiByteToWideChar(CP_UTF8, 0, source, sLen, result, rLen);
	result[rLen] = '\0';
	return result;
}