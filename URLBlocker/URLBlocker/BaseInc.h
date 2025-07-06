#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fwpmu.h>
#include <initguid.h>
#include <guiddef.h>
#include <ip2string.h>
#include <comdef.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <atomic>

#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "ws2_32.lib")