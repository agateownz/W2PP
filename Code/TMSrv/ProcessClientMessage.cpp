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
#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include <errno.h>

#include "..\Basedef.h"
#include "..\CPSock.h"
#include "..\ItemEffect.h"
#include "Language.h"

#include "CItem.h"
#include "Server.h"
#include "ProcessClientMessage.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "DumpStruct.h"

void  ProcessClientMessage(int conn, char *pMsg, BOOL isServer)
{
	MSG_STANDARD *std = (MSG_STANDARD *)pMsg;

	if ((std->ID < 0) || (std->ID >= MAX_USER)) 
	{	
		MSG_STANDARD *m = (MSG_STANDARD *)pMsg;

		sprintf(temp, "err,packet Type:%d ID:%d Size:%d KeyWord:%d", m->Type, m->ID, m->Size, m->KeyWord);

		Log(temp, "-system", 0);

		return;
	}

	if (ServerDown >= 120)
		return;

	if (conn > 0 && conn < MAX_USER)
		pUser[conn].LastReceiveTime = SecCounter;

	if (std->Type == _MSG_Ping) {
        W2::DumpPacket(STRINGIFY(_MSG_Ping), std, std->Size, W2::PacketDirection::CLIENT2SERVER, "Ping");
		return;
	}

	// Checa se o pacote foi enviado por algum jogador e possui o timestamp de controle interno.
	if (isServer == FALSE && std->ClientTick == SKIPCHECKTICK)
		return;

	switch(std->Type)
	{
	case _MSG_AccountLogin:
        W2::DumpPacket(STRINGIFY(_MSG_AccountLogin), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Account Login");
		Exec_MSG_AccountLogin(conn, pMsg);
		break;

	case _MSG_CharacterLogin:
        W2::DumpPacket(STRINGIFY(_MSG_CharacterLogin), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Character Login");
		Exec_MSG_CharacterLogin(conn, pMsg);
		break;

	case _MSG_CharacterLogout:
		W2::DumpPacket(STRINGIFY(_MSG_CharacterLogout), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Character Logout");
		Exec_MSG_CharacterLogout(conn, pMsg);
		break;

	case _MSG_DeleteCharacter:
        W2::DumpPacket(STRINGIFY(_MSG_DeleteCharacter), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Delete Character");
		Exec_MSG_DeleteCharacter(conn, pMsg);
		break;

	case _MSG_CreateCharacter:
        W2::DumpPacket(STRINGIFY(_MSG_CreateCharacter), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Create Character");
		Exec_MSG_CreateCharacter(conn, pMsg);
		break;

	case _MSG_AccountSecure:
        W2::DumpPacket(STRINGIFY(_MSG_AccountSecure), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Account Secure");
		Exec_MSG_AccountSecure(conn, pMsg);
		break;

	case _MSG_MessageChat:
        W2::DumpPacket(STRINGIFY(_MSG_MessageChat), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Message Chat");
		Exec_MSG_MessageChat(conn, pMsg);
		break;

	case _MSG_Action:
	case _MSG_Action2:
	case _MSG_Action3:
        W2::DumpPacket(STRINGIFY(_MSG_Action), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Action");
		Exec_MSG_Action(conn, pMsg);
		break;

	case _MSG_Motion:
        W2::DumpPacket(STRINGIFY(_MSG_Motion), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Motion");
		Exec_MSG_Motion(conn, pMsg);
		break;			

	case _MSG_UpdateScore:
	{
		Log("cra client send update score", pUser[conn].AccountName, pUser[conn].IP);
		AddCrackError(conn, 2, 91);
	} break;

	case _MSG_NoViewMob:
        W2::DumpPacket(STRINGIFY(_MSG_NoViewMob), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "No View Mob");
		Exec_MSG_NoViewMob(conn, pMsg);
		break;

	case _MSG_Restart:
        W2::DumpPacket(STRINGIFY(_MSG_Restart), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Restart");
		Exec_MSG_Restart(conn, pMsg);
		break;

	case _MSG_Deprivate:
        W2::DumpPacket(STRINGIFY(_MSG_Deprivate), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Deprivate");
		Exec_MSG_Deprivate(conn, pMsg);
		break;

	case _MSG_Challange:
        W2::DumpPacket(STRINGIFY(_MSG_Challange), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Challange");
		Exec_MSG_Challange(conn, pMsg);
		break;

	case _MSG_ChallangeConfirm:
        W2::DumpPacket(STRINGIFY(_MSG_ChallangeConfirm), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Challange Confirm");
		Exec_MSG_ChallangeConfirm(conn, pMsg);
		break;

	case _MSG_ReqTeleport:
        W2::DumpPacket(STRINGIFY(_MSG_ReqTeleport), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Req Teleport");
		Exec_MSG_ReqTeleport(conn, pMsg);
		break;

	case _MSG_REQShopList:
        W2::DumpPacket(STRINGIFY(_MSG_REQShopList), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "REQ Shop List");
		Exec_MSG_REQShopList(conn, pMsg);
		break;

	case _MSG_Deposit:
        W2::DumpPacket(STRINGIFY(_MSG_Deposit), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Deposit");
		Exec_MSG_Deposit(conn, pMsg);
		break;

	case _MSG_Withdraw:
        W2::DumpPacket(STRINGIFY(_MSG_Withdraw), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Withdraw");
		Exec_MSG_Withdraw(conn, pMsg);
		break;

	case _MSG_RemoveParty:
        W2::DumpPacket(STRINGIFY(_MSG_RemoveParty), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Remove Party");
		Exec_MSG_RemoveParty(conn, pMsg);
		break;

	case _MSG_SendReqParty:
        W2::DumpPacket(STRINGIFY(_MSG_SendReqParty), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Send Req Party");
		Exec_MSG_SendReqParty(conn, pMsg);
		break;

	case _MSG_AcceptParty:
        W2::DumpPacket(STRINGIFY(_MSG_AcceptParty), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Accept Party");
		Exec_MSG_AcceptParty(conn, pMsg);
		break;

	case _MSG_TradingItem:
        W2::DumpPacket(STRINGIFY(_MSG_TradingItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Trading Item");
		Exec_MSG_TradingItem(conn, pMsg);
		break;

	case _MSG_MessageWhisper:
        W2::DumpPacket(STRINGIFY(_MSG_MessageWhisper), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Message Whisper");
	    Exec_MSG_MessageWhisper(conn, pMsg);
		break;

	case _MSG_ChangeCity:
        W2::DumpPacket(STRINGIFY(_MSG_ChangeCity), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Change City");
		Exec_MSG_ChangeCity(conn, pMsg);
		break;

	case _MSG_PKMode:
        W2::DumpPacket(STRINGIFY(_MSG_PKMode), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "PK Mode");
		Exec_MSG_PKMode(conn, pMsg);
		break;

	case _MSG_ReqTradeList:
        W2::DumpPacket(STRINGIFY(_MSG_ReqTradeList), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Req Trade List");
		Exec_MSG_ReqTradeList(conn, pMsg);
		break;

	case _MSG_UpdateItem:
        W2::DumpPacket(STRINGIFY(_MSG_UpdateItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Update Item");
		Exec_MSG_UpdateItem(conn, pMsg);
		break;

	case _MSG_Quest:
        W2::DumpPacket(STRINGIFY(_MSG_Quest), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Quest");
		Exec_MSG_Quest(conn, pMsg);
		break;

	case _MSG_SetShortSkill:
        W2::DumpPacket(STRINGIFY(_MSG_SetShortSkill), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Set Short Skill");
		Exec_MSG_SetShortSkill(conn, pMsg);
		break;

	case _MSG_Attack:
	case _MSG_AttackOne:
	case _MSG_AttackTwo:
        W2::DumpPacket(STRINGIFY(_MSG_Attack), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Attack");
		Exec_MSG_Attack(conn, pMsg);
		break;

	case _MSG_DropItem:
        W2::DumpPacket(STRINGIFY(_MSG_DropItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Drop Item");
		Exec_MSG_DropItem(conn, pMsg);
		break;

	case _MSG_GetItem:
        W2::DumpPacket(STRINGIFY(_MSG_GetItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Get Item");
		Exec_MSG_GetItem(conn, pMsg);
		break;

	case _MSG_QuitTrade: 
        W2::DumpPacket(STRINGIFY(_MSG_QuitTrade), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Quit Trade");
		Exec_MSG_QuitTrade(conn, pMsg);
		break;

	case _MSG_UseItem:
        W2::DumpPacket(STRINGIFY(_MSG_UseItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Use Item");
		Exec_MSG_UseItem(conn, pMsg);
		break;

	case _MSG_ApplyBonus:
        W2::DumpPacket(STRINGIFY(_MSG_ApplyBonus), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Apply Bonus");
		Exec_MSG_ApplyBonus(conn, pMsg);
		break;

	case _MSG_SendAutoTrade:
        W2::DumpPacket(STRINGIFY(_MSG_SendAutoTrade), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Send Auto Trade");
		Exec_MSG_SendAutoTrade(conn, pMsg);
		break;

	case _MSG_ReqBuy:
        W2::DumpPacket(STRINGIFY(_MSG_ReqBuy), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Req Buy");
		Exec_MSG_ReqBuy(conn, pMsg);
		break;

	case _MSG_Buy:
        W2::DumpPacket(STRINGIFY(_MSG_Buy), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Buy");
		Exec_MSG_Buy(conn, pMsg);
		break;

	case _MSG_Sell:
        W2::DumpPacket(STRINGIFY(_MSG_Sell), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Sell");
		Exec_MSG_Sell(conn, pMsg);
		break;

	case _MSG_Trade:
        W2::DumpPacket(STRINGIFY(_MSG_Trade), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Trade");
		Exec_MSG_Trade(conn, pMsg);
		break;

	case _MSG_CombineItem:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item");
		Exec_MSG_CombineItem(conn, pMsg);
		break;

	case _MSG_ReqRanking:
        W2::DumpPacket(STRINGIFY(_MSG_ReqRanking), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Req Ranking");
		Exec_MSG_ReqRanking(conn, pMsg);
		break;

	case _MSG_CombineItemEhre:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemEhre), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Ehre");
		Exec_MSG_CombineItemEhre(conn, pMsg);
		break;

	case _MSG_CombineItemTiny:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemTiny), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Tiny");
		Exec_MSG_CombineItemTiny(conn, pMsg);
		break;

	case _MSG_CombineItemShany:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemShany), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Shany");
		Exec_MSG_CombineItemShany(conn, pMsg);
		break;

	case _MSG_CombineItemAilyn:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemAilyn), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Ailyn");
		Exec_MSG_CombineItemAilyn(conn, pMsg);
		break;

	case _MSG_CombineItemAgatha:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemAgatha), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Agatha");
		Exec_MSG_CombineItemAgatha(conn, pMsg);
		break;

	case _MSG_CombineItemOdin:
	case _MSG_CombineItemOdin2:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemOdin), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Odin");
		Exec_MSG_CombineItemOdin(conn, pMsg);
		break;

	case _MSG_DeleteItem:
        W2::DumpPacket(STRINGIFY(_MSG_DeleteItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Delete Item");
		Exec_MSG_DeleteItem(conn, pMsg);
		break;

	case _MSG_InviteGuild:
        W2::DumpPacket(STRINGIFY(_MSG_InviteGuild), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Invite Guild");
		Exec_MSG_InviteGuild(conn, pMsg);
		break;

	case  _MSG_SplitItem:
        W2::DumpPacket(STRINGIFY(_MSG_SplitItem), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Split Item");
		Exec_MSG_SplitItem(conn, pMsg);
		break;

	case _MSG_CombineItemLindy:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemLindy), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Lindy");
		Exec_MSG_CombineItemLindy(conn, pMsg);
		break;

	case _MSG_CombineItemAlquimia:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemAlquimia), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Alquimia");
		Exec_MSG_CombineItemAlquimia(conn, pMsg);
		break;

	case _MSG_CombineItemExtracao:
        W2::DumpPacket(STRINGIFY(_MSG_CombineItemExtracao), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Combine Item Extracao");
		Exec_MSG_CombineItemExtracao(conn, pMsg);
		break;

	case _MSG_GuildAlly:
        W2::DumpPacket(STRINGIFY(_MSG_GuildAlly), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Guild Ally");
		Exec_MSG_GuildAlly(conn, pMsg);
		break;

	case _MSG_War:
        W2::DumpPacket(STRINGIFY(_MSG_War), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "War");
		Exec_MSG_War(conn, pMsg);
		break;

	case _MSG_CapsuleInfo:
        W2::DumpPacket(STRINGIFY(_MSG_CapsuleInfo), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Capsule Info");
		Exec_MSG_CapsuleInfo(conn, pMsg);
		break;

	case _MSG_PutoutSeal:
        W2::DumpPacket(STRINGIFY(_MSG_PutoutSeal), pMsg, std->Size, W2::PacketDirection::CLIENT2SERVER, "Putout Seal");
		Exec_MSG_PutoutSeal(conn, pMsg);
		break;

	}
	return;
}
