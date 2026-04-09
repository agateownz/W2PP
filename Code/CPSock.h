/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

#ifndef _CPSOCK_ // Last updated Phase 3 - Socket Abstraction
#define _CPSOCK_ 

// Platform detection
#ifdef _WIN32
    #include <winsock2.h>
    #include <Windows.h>
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
    
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define closesocket close
    #define WSAGetLastError() errno
    
    // Windows message constants (for compatibility - not used on Linux)
    #define WM_USER 0x0400
    #define FD_ACCEPT 1
    #define FD_READ 2
    #define FD_WRITE 4
    #define FD_CLOSE 8
#endif

#include <memory>
#include "Common/ISocket.h"

// Socket event messages (for compatibility with Windows message loop)
#define WSA_READ            (WM_USER + 100)
#define WSA_READDB          (WM_USER + 2) 
#define WSA_ACCEPT          (WM_USER + 3) 
#define WSA_READBILL		(WM_USER + 4)
#define WSA_ACCEPTADMIN     (WM_USER + 5) 
#define WSA_READADMIN		(WM_USER + 6) 
#define WSA_READADMINCLIENT	(WM_USER + 7) 

#define MAX_PENDING_CONNECTS  8

#define RECV_BUFFER_SIZE      (128*1024)         // Maximum buffer size to receive messages of 64k
#define SEND_BUFFER_SIZE      (128*1024)         // Maximum buffer size to send messages of 64K

#define MAX_MESSAGE_SIZE           8192         // Maximum size a single message can have, 4K

#define INITCODE               0x1F11F311

typedef struct _HEADER
{
	short		  Size;
	char		  KeyWord;
	char		  CheckSum;
	short		  Type;
	short		  ID;
	unsigned int  ClientTick; 
} HEADER, *PHEADER;

// Forward declarations
namespace W2PP {
namespace Network {
    class SocketEventHandler;
}
}

class  CPSock
{   
public:
	unsigned int  Sock;
	char   *pSendBuffer;
	char   *pRecvBuffer;
	int		nSendPosition;
	int		nRecvPosition;
	int		nProcPosition;
	int		nSentPosition;
	int     Init;	

private:
    // New: Socket abstraction interface
    std::shared_ptr<ISocket> m_socketImpl;
    int m_wsaMessage;
#ifdef _WIN32
    HWND m_hWnd;
#else
    void* m_hWnd;
#endif

public:
	CPSock();
	~CPSock();

	BOOL	CloseSocket			();
	BOOL	WSAInitialize		();
	
    // Legacy signature for compatibility - hWnd and WSA are now optional/ignored
#ifdef _WIN32
    SOCKET	StartListen			(HWND hWnd, int ip, int Port, int WSA);
    SOCKET	ConnectServer		(char *HostAddr, int Port, int ip, int WSA);
    SOCKET  ConnectBillServer	(char *HostAddr, int Port, int ip, int WSA);
#else
    SOCKET	StartListen			(void* hWnd, int ip, int Port, int WSA);
    SOCKET	ConnectServer		(char *HostAddr, int Port, int ip, int WSA);
    SOCKET  ConnectBillServer	(char *HostAddr, int Port, int ip, int WSA);
#endif

	BOOL	Receive				();
	char   *ReadMessage			(int *ErrorCode, int *ErrorType);
	
	BOOL	AddMessage			(char *pMsg, int Size);
	BOOL	SendMessageA		();
	BOOL	SendOneMessage		(char* Msg, int Size);
	
	void	RefreshRecvBuffer	(void);
	void	RefreshSendBuffer	(void);
	BOOL	SendBillMessage		(char * Msg);
	char   *ReadBillMessage		(int *ErrorCode, int *ErrorType);

    // New methods for async event handling
    void SetEventCallback(int wsaMsg);
    void OnSocketEvent(int eventType, int errorCode);

private:
    void InitializeBuffers();
    void CleanupBuffers();
};

struct _AUTH_GAME // NEEDS TO BE FIXED ACCORDING TO WYD 1.2 6.13 SIZE IS 0xC4 (196)
{
	char Unk[196];	
};

#define g_cGame  (sizeof(_AUTH_GAME))

// C-style helper functions for socket operations
extern "C" {
    // Initialize socket library
    int Socket_Initialize();
    
    // Cleanup socket library
    void Socket_Cleanup();
    
    // Get last socket error
    int Socket_GetLastError();
    
    // Set socket to non-blocking mode
    int Socket_SetNonBlocking(SOCKET sock);
    
    // Check if socket has data available (non-blocking check)
    int Socket_HasData(SOCKET sock);
}

#endif
