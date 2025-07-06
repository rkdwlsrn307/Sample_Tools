#include "WFPManagerInc.h"

WFPManager::WFPManager() : m_engine(NULL), m_defaultFilter(NULL) {}
WFPManager::~WFPManager()
{
    FinalWFPManager();
}

void WFPManager::InitWFPManager()
{
    HANDLE threadHandle;
    DWORD threadId;
    FWPM_SESSION0 session = {};
    session.displayData.name = const_cast<wchar_t*>(L"URLBlocker");
    session.flags = FWPM_SESSION_FLAG_DYNAMIC;

    if (FwpmEngineOpen0(nullptr, RPC_C_AUTHN_WINNT, nullptr, &session, &m_engine) != ERROR_SUCCESS)
        std::cerr << "FwpmEngineOpen0 failed. Error: " << GetLastError() << std::endl;

    threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WFPManagerThread, this, 0, &threadId);

    CloseHandle(threadHandle);
}

void WFPManager::FinalWFPManager()
{
    SubDefaultFilter();

    if (m_engine) {
        FwpmEngineClose0(m_engine);
        m_engine = nullptr;
    }
}

void WFPManager::WFPManagerThread()
{
    WFPManager::Get().DoWFPManagerThread();
}

void WFPManager::DoWFPManagerThread()
{
    while (1)
    {
        Sleep(100);
    }
}

void WFPManager::AddDefaultFilter()
{
    FWPM_FILTER0 filter = {};
    filter.displayData.name = const_cast<wchar_t*>(L"DefaultBlockUrl");
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    filter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
    filter.action.type = FWP_ACTION_BLOCK;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = 0;

    if (FwpmFilterAdd0(m_engine, &filter, nullptr, &m_defaultFilter) != ERROR_SUCCESS)
        return;
}

void WFPManager::SubDefaultFilter()
{
    if (m_defaultFilter == 0)
        return;

    if (FwpmFilterDeleteById0(m_engine, m_defaultFilter) != ERROR_SUCCESS)
        std::cerr << "FwpmFilterDeleteById0 failed. Error: " << GetLastError() << std::endl;
}

void WFPManager::AddUrl(const std::string& url)
{
    std::string addIp = ResolveDomainToIp(url);
    FWPM_FILTER0 filter = {};
    FWPM_FILTER_CONDITION0 cond = {};
    IN_ADDR addr;

    if (InetPtonA(AF_INET, addIp.c_str(), &addr) != 1) {
        std::cerr << "Invalid IP" << std::endl;
    }

    filter.displayData.name = const_cast<wchar_t*>(L"AllowInputIP");
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    filter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = 10;

    cond.fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
    cond.matchType = FWP_MATCH_EQUAL;
    cond.conditionValue.type = FWP_UINT32;
    cond.conditionValue.uint32 = htonl(addr.S_un.S_addr);

    filter.filterCondition = &cond;
    filter.numFilterConditions = 1;

    if (FwpmFilterAdd0(m_engine, &filter, nullptr, nullptr) != ERROR_SUCCESS)
    {
        return;
    }

    AddDefaultFilter();
}

std::string WFPManager::ResolveDomainToIp(const std::string& domain)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    addrinfo hints = {}, * res = nullptr;
    hints.ai_family = AF_UNSPEC;

    int err = getaddrinfo(domain.c_str(), nullptr, &hints, &res);
    if (err != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerrorA(err) << std::endl;
        return "";
    }

    char ipBuf[INET6_ADDRSTRLEN] = {};

    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            sockaddr_in* ipv4 = (sockaddr_in*)p->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ipBuf, sizeof(ipBuf));
            break;
        }
    }

    freeaddrinfo(res);
    WSACleanup();

    return std::string(ipBuf);
}