#pragma once

class WFPManager
{
private:
	UINT64 m_defaultFilter;
	std::map<std::string, UINT64> m_filters;
	HANDLE m_engine;

private:
	void AddDefaultFilter();
	void SubDefaultFilter();
	std::string ResolveDomainToIp(const std::string& domain);

public:
	void AddUrl(const std::string& url);
	void SubUrl(std::string url);

public:
	static void WINAPI WFPManagerThread();
	void DoWFPManagerThread();

public:
	void InitWFPManager();
	void FinalWFPManager();

public:
	WFPManager();
	~WFPManager();

	static WFPManager& Get()
	{
		static WFPManager instance;

		return instance;
	}
};