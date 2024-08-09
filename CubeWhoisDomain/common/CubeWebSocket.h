#pragma once

class CubeWebSocket
{
public:

private:
	WSADATA m_wsa_data;
	SOCKET m_socket;
	int m_ret;

	addrinfo
		* result = NULL,
		* ptr = NULL,
		hints;

private:
	const char* m_data;

public:
	CubeWebSocket();
	~CubeWebSocket();

public:
	void Initialize();
	void Finalize();
	void SendToSocket(char *send_data);

private:
	INT InitializeSocket();
	INT CreateSocket();
	INT ConnectSocket();
	void ShutDownToSocket();
	void ReciveToSocket();

private:

public:
	static CubeWebSocket& Get()
	{
		static CubeWebSocket instance;
		return instance;
	}
};