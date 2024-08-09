#include "BaseNetworkInc.h"

int main()
{
    std::cout << "[WelCome to CubeWhois]" << std::endl;
    std::cout << " Help Command -> /help" << std::endl;

    CubeWebSocket::Get().Initialize();
    CubeWhoisDomain::Get().Initialize();

    while (TRUE)
    {
        Sleep(1000);
    }

    CubeWebSocket::Get().~CubeWebSocket();

    return 0;
}