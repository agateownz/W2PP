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

#include "CUser.h"
#include "Common/SocketEventHandler.h"

#ifdef _WIN32
extern HWND hWndMain; // Server.cpp
#endif

CUser::CUser()
{
	Mode = USER_EMPTY;
	IP   = 0;
	Count = 0;
	DisableID = 0;
	Level = -1;
}

CUser::~CUser()
{
}

BOOL CUser::AcceptUser(int ListenSocket, int wsa)
{
	SOCKADDR_IN acc_sin; 
	socklen_t Size = sizeof(acc_sin);

	SOCKET tSock = accept(ListenSocket, (struct sockaddr *)&acc_sin, &Size);

	if(tSock == INVALID_SOCKET) 
		return FALSE;

    // Set non-blocking mode
    Socket_SetNonBlocking(tSock);

    // Register for async events with the new socket event handler
    auto& handler = W2PP::Network::SocketEventHandler::GetInstance();
    if (handler.IsRunning())
    {
        handler.RegisterSocket(tSock, wsa);
    }

	cSock.Sock          = (unsigned int)tSock;
	cSock.nRecvPosition = 0;
	cSock.nProcPosition = 0;
	cSock.nSendPosition = 0;

	IP		   = acc_sin.sin_addr.s_addr;
	Mode	   = USER_ACCEPT;

	return TRUE;
}
