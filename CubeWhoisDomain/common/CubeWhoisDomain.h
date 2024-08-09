#pragma once

class CubeWhoisDomain
{
private:
	BOOL m_init_whois_thread;

private:
	void initCommandThread();
	void DoCommandThread();
	void SendCommandFunction();
	void WhoisDomainSend(std::string data);

private:
	static void WINAPI CommandThread();

public:
	void Initialize();
	void Finalize();

public:
	CubeWhoisDomain();
	~CubeWhoisDomain();

public:
	static CubeWhoisDomain& Get()
	{
		static CubeWhoisDomain instance;
		return instance;
	}

};