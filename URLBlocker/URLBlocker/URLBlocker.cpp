#include "WFPManagerInc.h"

int main()
{
    WFPManager::Get().InitWFPManager();

    while (1)
    {
        std::string url;
        std::getline(std::cin, url);

        if (url == "exit" || url.empty())
            break;

        WFPManager::Get().AddUrl(url);
        Sleep(100);
    }

    return 0;
}
