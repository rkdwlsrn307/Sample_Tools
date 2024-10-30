#pragma once

#include<string>
#include<iostream>
#include<fstream>
#include<sstream>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <mutex>
#include <string>

#include <oleacc.h>
#include <tchar.h>
#include <comutil.h>
#include <strsafe.h>
#include <time.h>
#include <windows.h>
#include <Winternl.h>
#include <sys/stat.h>
#include <Shlobj.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include "UIAutomation.h"

#pragma comment(lib, "oleacc.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Winmm.lib") 

#pragma warning( disable : 4995 )
#pragma warning( disable : 4267 )

typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)
(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
);

#include "WebUIAutomation.h"