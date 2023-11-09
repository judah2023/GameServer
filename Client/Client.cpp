// Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	//----------------------
	// Initialize Winsock
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != ERROR_SUCCESS) // ERROR_SUCCESS = 0L
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		std::cout << "WSAStartup failed with error: " << err << "\n";
		return 1;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
		HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))	// Checking Version
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		std::cout << "Could not find a usable version of Winsock.dll\n";
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "The Winsock 2.2 dll was found okay\n";
	}

	/* The Winsock DLL is acceptable. Proceed to use it. */

	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connectSocket == INVALID_SOCKET)
	{
		std::cout << "socket function failed with error = " << WSAGetLastError() << "\n";
		WSACleanup();
		return 1;
	}

	std::cout << "socket function succeeded\n";
	
	// IPv4

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	SOCKADDR_IN clientService{ 0 };

	clientService.sin_family = AF_INET;						// AF_INET : IPv4
	inet_pton(AF_INET, "127.0.0.1", &clientService.sin_addr); // IP 127.0.0.1 : MyCom IP
	// htons : host to network short
	clientService.sin_port = htons(7777);						// port : 7777

	//----------------------
	// Connect to server.
	err = connect(connectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (err == SOCKET_ERROR)
	{
		std::cout << "connect function failed with error = " << WSAGetLastError() << "\n";

		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Connected to server.\n";

	while (true)
	{
		std::cout << "Connecting...\n";
		Sleep(100);
	}


	/* Add network programming using Winsock here */

	closesocket(connectSocket);
	WSACleanup();
    std::cin.get();	// Prevent Exit Window
    return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
