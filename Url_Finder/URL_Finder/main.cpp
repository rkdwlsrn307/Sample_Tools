#include "BaseInc.h"

int main()
{
	WebUIAutomtaion::Get().Initialize();
	
	while (TRUE)
	{
		Sleep(1000);
	}

	WebUIAutomtaion::Get().~WebUIAutomtaion();

	return 0;
}