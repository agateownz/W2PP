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
#include "Server.h"
#include "Common/SocketEventHandler.h"


CUser::CUser()
{	
	Mode		     = USER_EMPTY;
	Unk3			 = 0;
	IsBillConnect    = 0;
	LastReceiveTime  = 0;

	memset(Cargo, 0, sizeof(Cargo));

	Admin			 = 0;
	CastleStatus	 = 0;
	MuteChat		 = 0;
	UseItemTime		 = 0;
	Message			 = 0;
	AttackTime		 = 0;
	LastClientTick	 = 0;
	PotionTime		 = 0;

	OnlyTrade		 = 0;
}

CUser::~CUser()
{

}

BOOL CUser::AcceptUser(int ListenSocket)
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
        handler.RegisterSocket(tSock, WSA_READ);
    }

	cSock.Sock          = (unsigned int)tSock;
	cSock.nRecvPosition = 0;
	cSock.nProcPosition = 0;
	cSock.nSendPosition = 0;

	IP		   = acc_sin.sin_addr.s_addr;
	Mode	   = USER_ACCEPT;
	Unk3	   = 0;

	return TRUE;
}

int CUser::CloseUser()
{
     cSock.CloseSocket();
	 cSock.Sock		= 0;
	 IsBillConnect  = 0;
	 
	 Mode = USER_EMPTY;

	 AccountName[0] = 0;

	 return TRUE;
}
