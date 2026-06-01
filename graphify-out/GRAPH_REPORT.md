# Graph Report - .  (2026-06-01)

## Corpus Check
- 111 files · ~179,892 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 739 nodes · 2068 edges · 51 communities (39 shown, 12 thin omitted)
- Extraction: 53% EXTRACTED · 47% INFERRED · 0% AMBIGUOUS · INFERRED: 966 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Base Item & Mob Helpers|Base Item & Mob Helpers]]
- [[_COMMUNITY_Item Crafting & Trade|Item Crafting & Trade]]
- [[_COMMUNITY_Data Initialization & IO|Data Initialization & I/O]]
- [[_COMMUNITY_DB Server Core|DB Server Core]]
- [[_COMMUNITY_Castle & Config Systems|Castle & Config Systems]]
- [[_COMMUNITY_Player Commerce & Actions|Player Commerce & Actions]]
- [[_COMMUNITY_Combat & Battle Logic|Combat & Battle Logic]]
- [[_COMMUNITY_Networking & Sessions|Networking & Sessions]]
- [[_COMMUNITY_File Database Engine|File Database Engine]]
- [[_COMMUNITY_Guild & World Features|Guild & World Features]]
- [[_COMMUNITY_Broadcast Functions|Broadcast Functions]]
- [[_COMMUNITY_Account & Server Utils|Account & Server Utils]]
- [[_COMMUNITY_Social & Account Systems|Social & Account Systems]]
- [[_COMMUNITY_Project Documentation|Project Documentation]]
- [[_COMMUNITY_Main Server Loop|Main Server Loop]]
- [[_COMMUNITY_Mob AI & Pathing|Mob AI & Pathing]]
- [[_COMMUNITY_Guild War System|Guild War System]]
- [[_COMMUNITY_World Event Systems|World Event Systems]]
- [[_COMMUNITY_Ranking System|Ranking System]]
- [[_COMMUNITY_Player Interactions|Player Interactions]]
- [[_COMMUNITY_NPC Generation|NPC Generation]]
- [[_COMMUNITY_Damage Calculation|Damage Calculation]]
- [[_COMMUNITY_Client Patch|Client Patch]]
- [[_COMMUNITY_Build System|Build System]]
- [[_COMMUNITY_Ranking Class|Ranking Class]]
- [[_COMMUNITY_Billing & Logout|Billing & Logout]]
- [[_COMMUNITY_NPC Generator Class|NPC Generator Class]]
- [[_COMMUNITY_Socket Class|Socket Class]]
- [[_COMMUNITY_FileDB Class|FileDB Class]]
- [[_COMMUNITY_DB User Class|DB User Class]]
- [[_COMMUNITY_Item Class Header|Item Class Header]]
- [[_COMMUNITY_Mob Class Header|Mob Class Header]]
- [[_COMMUNITY_ReadFiles Header|ReadFiles Header]]
- [[_COMMUNITY_TM User Class|TM User Class]]
- [[_COMMUNITY_War Tower Class|War Tower Class]]
- [[_COMMUNITY_DB ReadFiles Class|DB ReadFiles Class]]
- [[_COMMUNITY_OpenCode Config|OpenCode Config]]
- [[_COMMUNITY_OpenCode Package|OpenCode Package]]
- [[_COMMUNITY_Castle Zakum Class|Castle Zakum Class]]

## God Nodes (most connected - your core abstractions)
1. `GridMulticast()` - 67 edges
2. `SendClientMessage()` - 63 edges
3. `ProcessClientMessage()` - 61 edges
4. `ProcessSecTimer()` - 58 edges
5. `Exec_MSG_UseItem()` - 58 edges
6. `Exec_MSG_Attack()` - 42 edges
7. `Exec_MSG_Quest()` - 39 edges
8. `ProcessImple()` - 38 edges
9. `SendItem()` - 37 edges
10. `BASE_GetItemAbility()` - 36 edges

## Surprising Connections (you probably didn't know these)
- `WriteGuild()` --calls--> `BASE_GetGuildName()`  [INFERRED]
  Code/TMSrv/CReadFiles.cpp → Code/Basedef.cpp
- `Exec_MSG_Attack()` --calls--> `BASE_GetManaSpent()`  [INFERRED]
  Code/TMSrv/_MSG_Attack.cpp → Code/Basedef.cpp
- `CheckMove()` --calls--> `DoTeleport()`  [INFERRED]
  Code/TMSrv/CCastleZakum.cpp → Code/TMSrv/Server.cpp
- `ProcessSecTimer()` --calls--> `GetInHalf()`  [INFERRED]
  Code/TMSrv/ProcessSecMinTimer.cpp → Code/TMSrv/GetFunc.cpp
- `Exec_MSG_Quest()` --calls--> `SetCurKill()`  [INFERRED]
  Code/TMSrv/_MSG_Quest.cpp → Code/TMSrv/GetFunc.cpp

## Import Cycles
- None detected.

## Communities (51 total, 12 thin omitted)

### Community 0 - "Base Item & Mob Helpers"
Cohesion: 0.06
Nodes (75): BASE_CanCargo(), BASE_CanCarry(), BASE_CanTrade(), BASE_CheckItemDate(), BASE_GetBonusItemAbility(), BASE_GetBonusItemAbilityNosanc(), BASE_GetCurrentScore(), BASE_GetGrowthRate() (+67 more)

### Community 1 - "Item Crafting & Trade"
Cohesion: 0.09
Nodes (36): BASE_GetItemAbilityNosanc(), BASE_GetItemCode(), BASE_GetLanguage(), BASE_SetItemSanc(), STRUCT_ITEM, MSG_UpdateItem, OpenCastleGate(), Exec_MSG_CombineItem() (+28 more)

### Community 2 - "Data Initialization & I/O"
Cohesion: 0.07
Nodes (43): BASE_ApplyAttribute(), BASE_CanEquip(), BASE_ClearMob(), BASE_ClearMobExtra(), BASE_GetAccuracyRate(), BASE_GetArena(), BASE_GetBonusScorePoint(), BASE_GetBonusSkillPoint() (+35 more)

### Community 3 - "DB Server Core"
Cohesion: 0.10
Nodes (39): BASE_InitializeServerList(), BOOL, FILE, HANDLE, HINSTANCE, HWND, LONG, LPSTR (+31 more)

### Community 4 - "Castle & Config Systems"
Cohesion: 0.09
Nodes (21): BOOL, STRUCT_ITEM, CReadFiles(), BOOL, CheckMove(), KeyDrop(), ParseCastleString(), ProcessMinTimer() (+13 more)

### Community 5 - "Player Commerce & Actions"
Cohesion: 0.09
Nodes (21): BASE_CheckValidString(), GetItemPointer(), GetInView(), Exec_MSG_Buy(), Exec_MSG_CreateCharacter(), Exec_MSG_Deposit(), Exec_MSG_DropItem(), Exec_MSG_Motion() (+13 more)

### Community 6 - "Combat & Battle Logic"
Cohesion: 0.13
Nodes (32): MSG_Action, STRUCT_MOB, MSG_Attack, GetAction(), GetParryRate(), Exec_MSG_Attack(), Exec_MSG_Deprivate(), ProcessSecTimer() (+24 more)

### Community 7 - "Networking & Sessions"
Cohesion: 0.10
Nodes (21): BASE_CheckPacket(), BOOL, MSG_STANDARD, AddMessage(), CloseSocket(), ConnectBillServer(), ConnectServer(), BOOL (+13 more)

### Community 8 - "File Database Engine"
Cohesion: 0.14
Nodes (26): BASE_GetFirstKey(), STRUCT_SELCHAR, AddAccount(), AddAccountList(), CreateCharacter(), DBExportAccount(), DBGetSelChar(), DBReadAccount() (+18 more)

### Community 9 - "Guild & World Features"
Cohesion: 0.10
Nodes (27): BASE_CheckFairyDate(), BASE_ClearItem(), BASE_GetSubGuild(), BASE_GetVillage(), BASE_GetWeekNumber(), SetTotKill(), ProcessImple(), Exec_MSG_ChangeCity() (+19 more)

### Community 10 - "Broadcast Functions"
Cohesion: 0.15
Nodes (23): MSG_STANDARD, MobKilled(), Exec_MSG_NoViewMob(), GridMulticast(), MapaMulticast(), PartyGridMulticast(), SendChat(), SendClientSignalShortParm2() (+15 more)

### Community 11 - "Account & Server Utils"
Cohesion: 0.10
Nodes (19): CMob, FILE, STRUCT_SELCHAR, HFONT__, Exec_MSG_AccountLogin(), ApplyHp(), ApplyMp(), CheckFailAccount() (+11 more)

### Community 12 - "Social & Account Systems"
Cohesion: 0.08
Nodes (14): BOOL, Exec_MSG_AcceptParty(), Exec_MSG_AccountSecure(), Exec_MSG_CapsuleInfo(), Exec_MSG_CharacterLogin(), Exec_MSG_DeleteCharacter(), Exec_MSG_GuildAlly(), Exec_MSG_PKMode() (+6 more)

### Community 13 - "Project Documentation"
Cohesion: 0.10
Nodes (23): Free Software Foundation, Active Community, AttributeMap_Editor, BISrv, C/C++, ClientPatch_v7662, DBSrv, DropTool (+15 more)

### Community 14 - "Main Server Loop"
Cohesion: 0.12
Nodes (22): _AUTH_GAME, BASE_InitializeHitRate(), BOOL, HANDLE, HINSTANCE, HWND, LONG, LPSTR (+14 more)

### Community 15 - "Mob AI & Pathing"
Cohesion: 0.19
Nodes (14): BASE_GetRoute(), BASE_GetSpeed(), BattleProcessor(), CheckGetLevel(), GetEnemyFromView(), GetNextPos(), GetRandomPos(), GetTargetPos() (+6 more)

### Community 16 - "Guild War System"
Cohesion: 0.15
Nodes (18): BASE_GetGuildName(), tm, GuildProcess(), MobKilled(), Exec_MSG_ChallangeConfirm(), Exec_MSG_RemoveParty(), ProcessDBMessage(), SendClientSignalParm3() (+10 more)

### Community 17 - "World Event Systems"
Cohesion: 0.19
Nodes (18): BASE_UpdateItem(), MSG_CreateItem, GetCreateItem(), ClearArea(), ClearAreaGuild(), ClearChallanger(), DeleteColoseum(), DeleteGenerateMob() (+10 more)

### Community 18 - "Ranking System"
Cohesion: 0.30
Nodes (15): GrindRanking(), RankingSystem(), getElement(), getElementConnId(), increaseRankingElementValue(), loadRanking(), readAccountsInDir(), riseRankingElement() (+7 more)

### Community 19 - "Player Interactions"
Cohesion: 0.17
Nodes (12): MobKilled(), GetTeleportPosition(), Exec_MSG_ApplyBonus(), Exec_MSG_Challange(), Exec_MSG_InviteGuild(), Exec_MSG_MessageChat(), Exec_MSG_ReqTeleport(), SendClientMessage() (+4 more)

### Community 20 - "NPC Generation"
Cohesion: 0.19
Nodes (8): BASE_GetEnglish(), BASE_InitModuleDir(), STRUCT_MOB, Initialize(), ParseString(), ReadNPCGenerator(), SetAct(), ReadMob()

### Community 22 - "Damage Calculation"
Cohesion: 0.32
Nodes (8): BASE_GetDamage(), BASE_GetDistance(), MSG_Attack, MSG_AttackOne, GetAttack(), GetAttackArea(), GetAttribute(), ProcessAffect()

### Community 23 - "Client Patch"
Cohesion: 0.38
Nodes (6): DllMain(), strip_odd_connection_request(), strip_xtrap(), HINSTANCE, DWORD, LPVOID

### Community 24 - "Build System"
Cohesion: 0.33
Nodes (3): ClientPatch_v7662, DBSrv, TMSrv

### Community 25 - "Ranking Class"
Cohesion: 0.50
Nodes (3): class, GrindRanking(), RankingSystem()

### Community 26 - "Billing & Logout"
Cohesion: 0.40
Nodes (4): SaveAll(), Exec_MSG_CharacterLogout(), CharLogOut(), SendBilling()

### Community 27 - "NPC Generator Class"
Cohesion: 0.67
Nodes (3): class, CNPCGenerator(), CNPCSummon()

## Knowledge Gaps
- **75 isolated node(s):** `$schema`, `plugin`, `@opencode-ai/plugin`, `STRUCT_AFFECT`, `MSG_Action` (+70 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **12 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `SendClientMessage()` connect `Player Interactions` to `Base Item & Mob Helpers`, `Item Crafting & Trade`, `Player Commerce & Actions`, `Combat & Battle Logic`, `Guild & World Features`, `Broadcast Functions`, `Account & Server Utils`, `Social & Account Systems`, `Main Server Loop`, `Mob AI & Pathing`, `Guild War System`?**
  _High betweenness centrality (0.070) - this node is a cross-community bridge._
- **Why does `ProcessClientMessage()` connect `Social & Account Systems` to `Base Item & Mob Helpers`, `Item Crafting & Trade`, `Castle & Config Systems`, `Player Commerce & Actions`, `Combat & Battle Logic`, `Guild & World Features`, `Broadcast Functions`, `Account & Server Utils`, `Mob AI & Pathing`, `Guild War System`, `Player Interactions`, `Billing & Logout`?**
  _High betweenness centrality (0.066) - this node is a cross-community bridge._
- **Why does `MainWndProc()` connect `Main Server Loop` to `Data Initialization & I/O`, `Networking & Sessions`, `Account & Server Utils`, `Guild War System`, `Player Interactions`, `Billing & Logout`?**
  _High betweenness centrality (0.062) - this node is a cross-community bridge._
- **Are the 50 inferred relationships involving `GridMulticast()` (e.g. with `OpenCastleGate()` and `ProcessImple()`) actually correct?**
  _`GridMulticast()` has 50 INFERRED edges - model-reasoned connections that need verification._
- **Are the 56 inferred relationships involving `SendClientMessage()` (e.g. with `MobKilled()` and `OpenCastleGate()`) actually correct?**
  _`SendClientMessage()` has 56 INFERRED edges - model-reasoned connections that need verification._
- **Are the 59 inferred relationships involving `ProcessClientMessage()` (e.g. with `Exec_MSG_AcceptParty()` and `Exec_MSG_AccountLogin()`) actually correct?**
  _`ProcessClientMessage()` has 59 INFERRED edges - model-reasoned connections that need verification._
- **Are the 57 inferred relationships involving `ProcessSecTimer()` (e.g. with `BASE_CheckFairyDate()` and `BASE_CheckItemDate()`) actually correct?**
  _`ProcessSecTimer()` has 57 INFERRED edges - model-reasoned connections that need verification._