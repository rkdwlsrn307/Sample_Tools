#include "BaseNetworkInc.h"

CubeWhoisDomain::CubeWhoisDomain() :
	m_init_whois_thread(FALSE)
{
}

CubeWhoisDomain::~CubeWhoisDomain()
{
}

void CubeWhoisDomain::Initialize()
{
	initCommandThread();
}

void CubeWhoisDomain::Finalize()
{
	m_init_whois_thread = FALSE;
}

void CubeWhoisDomain::initCommandThread()
{
	HANDLE hthread;
	DWORD dwthread;

	m_init_whois_thread = TRUE;

	hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CommandThread, NULL, 0, &dwthread);

	CloseHandle(hthread);
}

void WINAPI CubeWhoisDomain::CommandThread()
{
	CubeWhoisDomain::Get().DoCommandThread();
}

void CubeWhoisDomain::DoCommandThread()
{
	while (m_init_whois_thread)
	{
		SendCommandFunction();
	}
}

void CubeWhoisDomain::SendCommandFunction()
{
	std::string data;

	std::cout << "CubeWhoisDomain : ";
	std::cin >> data;

	if (data.empty())
	{
		return;
	}
	else if (data.find("/") != std::string::npos)
	{
		if (data.find("clr") != std::string::npos)
		{
			system("cls");
		}
		else if (data.find("help") != std::string::npos)
		{
			std::cout << "Command List :" << std::endl;
			std::cout << "<-------------------------------------------->" << std::endl;
			std::cout << " /help" << std::endl;
			std::cout << " /clr" << std::endl;
			std::cout << "<-------------------------------------------->" << std::endl;
			std::cout << "To be Continue" << std::endl;
		}
		else if (data.find("reconnect") != std::string::npos)
		{
			std::cout << "ReConnect Socket" << std::endl;
		}
		else
		{
			std::cout << "Do not find Cammand -> /help" << std::endl;
		}
	}
	else
	{
		data.append("\r\n"); //Whois 서버로 도메인 전달시 해당 문자열이 없을경우 실패
		WhoisDomainSend(data);
	}
}

void CubeWhoisDomain::WhoisDomainSend(std::string data)
{
	char* send_data = new char[data.size()];

	std::copy(data.begin(), data.end(), send_data);

	CubeWebSocket::Get().SendToSocket(send_data);

	delete[] send_data;
}