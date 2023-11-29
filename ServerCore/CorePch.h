#pragma once
#include <iostream>
#include <memory>
using namespace std;

// stl
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

// socket
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h> 

// thread
#include <thread>  
#include <mutex>
#include <shared_mutex>

// macros
#define DATA_BUFSIZE 4096
#define GS_LOG() {cout << "Running...\t" << __FUNCTION__ << "(" << __LINE__ << ")\n";}
