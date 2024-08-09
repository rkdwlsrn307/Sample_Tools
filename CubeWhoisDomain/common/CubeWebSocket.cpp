#include "BaseNetworkInc.h"

CubeWebSocket::CubeWebSocket() :
	m_socket(INVALID_SOCKET), m_ret(NULL)
{
}

CubeWebSocket::~CubeWebSocket()
{
	Finalize();
}

void CubeWebSocket::Initialize()
{
	InitializeSocket();
}

void CubeWebSocket::Finalize()
{
	closesocket(m_socket);

	if (m_socket == INVALID_SOCKET)
	{
		m_socket = INVALID_SOCKET;
	}

	WSACleanup();
}

INT CubeWebSocket::InitializeSocket()
{
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (WSAStartup(MAKEWORD(2, 2), &m_wsa_data) != NULL)
	{
		std::cout << "[FAIL WSAStartup] errCode : " << WSAGetLastError() << std::endl;
		return 1;
	}

	return NULL;
}

INT CubeWebSocket::CreateSocket()
{
	if (getaddrinfo(CUBE_ADDRESS, CUBE_PORT, &hints, &result) != NULL)
	{
		WSACleanup();
		return 1;
	}

	ptr = result;

	m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (m_socket == INVALID_SOCKET)
	{
		std::cout << "[FAIL CreateSocket] errCode : " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		return 1;
	}

	return NULL;
}

INT CubeWebSocket::ConnectSocket()
{
	if (connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
	{
		std::cout << "[FAIL ConnectSocket] errCode : " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET)
	{
		return 1;
	}

	return NULL;
}

void CubeWebSocket::SendToSocket(char *send_data)
{
	if (m_socket == INVALID_SOCKET)
	{
		CreateSocket();
		ConnectSocket();
	}

	if (send_data == "")
	{
		std::cout << "[Send Data] Empty Send Data" << std::endl;
		return;
	}

	m_ret = send(m_socket, send_data, (int)strlen(send_data), 0);

	if (m_ret == SOCKET_ERROR)
	{
		std::cout << "[FAIL Send Data] errCode : " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return;
	}

	std::cout << "SUCESS send_data" << std::endl;

	ShutDownToSocket();
}

void CubeWebSocket::ShutDownToSocket()
{
	m_ret = shutdown(m_socket, SD_SEND);

	if (m_ret == SOCKET_ERROR)
	{
		std::cout << "[FAIL ShutDown] errCode : " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	ReciveToSocket();
}

void CubeWebSocket::ReciveToSocket()
{
	char recvbuf[CUBE_DEFAULT_BUFLEN];
	int recvbuflen = CUBE_DEFAULT_BUFLEN;

	do
	{
		m_ret = recv(m_socket, recvbuf, recvbuflen, 0);

		if (m_ret > 0)
		{
			std::cout << recvbuf << std::endl;
		}
		else if (m_ret == 0)
		{
			std::cout << "ShutDown Socket" << std::endl;
		}
		else if(m_ret < 0)
		{
			std::cout << "[FAIL Recive] errCode : " << WSAGetLastError() << std::endl;
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}

	} while (m_ret > 0);

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	return;
}