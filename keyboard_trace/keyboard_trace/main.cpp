#include "BaseInc.h"

int main()
{
	CKeyBoard::Get().Initialize();

	while (TRUE)
	{
		Sleep(1000);
	}

	CKeyBoard::Get().~CKeyBoard();

	return NULL;
}