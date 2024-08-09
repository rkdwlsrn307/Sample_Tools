#pragma once
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "zconf.h"
#include "zlib.h"

#pragma comment(lib, "Ws2_32.lib")

#define CUBE_PORT "43"
#define CUBE_ADDRESS "whois.verisign-grs.com"
#define CUBE_DEFAULT_BUFLEN 4096

#include "CubeWebSocket.h"
#include "CubeWhoisDomain.h"