#pragma once
#include <iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>  
#include <mswsock.h> 


#define DATA_BUFSIZE 4096
#define GS_LOG() {cout << "Running...\t" << __FUNCTION__ << "(" << __LINE__ << ")\n";}
