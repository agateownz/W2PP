## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

When the user types `/graphify`, invoke the `skill` tool with `skill: "graphify"` before doing anything else.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- Dirty graphify-out/ files are expected after hooks or incremental updates; dirty graph files are not a reason to skip graphify. Only skip graphify if the task is about stale or incorrect graph output, or the user explicitly says not to use it.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).

---

## W2PP Project Context

This is a reverse-engineered MMORPG server emulator for **WYD (With Your Destiny)**. It is a C/C++ Windows project.

### Build System

- **Visual Studio solution**: `W2PP Code Project.sln` (MSBuild, Win32 only).
- **Toolset**: v143 (VS2022), originally VS2015. WindowsTargetPlatformVersion is `10.0`.
- **Configurations**: `Debug|Win32`, `Release|Win32`, `Release-Themida|Win32`. The Themida config is only relevant for `ClientPatch_v7662`; for `TMSrv` and `DBSrv` it maps to `Release`.
- **Build output**: `release/<ProjectName>/run/` (not `Debug/` or `Release/` next to the project files). For example, `release/TMSrv/run/TMSrv.exe`.
- **No automated tests** exist in the repo. Do not attempt to run `ctest`, `make test`, etc.

### Project Boundaries

The solution contains three projects. Ignore `ClientPatch_v7662` per repo convention.

| Project | Type | Role |
|---------|------|------|
| `TMSrv` | Win32 GUI Application (`_WINDOWS` subsystem) | Game server. Handles client connections, game logic, mobs, combat, items, guilds, and world events. |
| `DBSrv` | Win32 GUI Application (`_WINDOWS` subsystem) | Database/account server. Handles account login, character data, file-based persistence, and admin-tool connections. |
| `ClientPatch_v7662` | DLL (Debug) / Application (Release) | Client-side patch. **Ignore this.** |

### Shared Code

Both server projects compile the same shared sources directly into their respective binaries:
- `Code/Basedef.cpp` and `Code/Basedef.h` — core structs, constants, message definitions, and utility functions.
- `Code/CPSock.cpp` and `Code/CPSock.h` — custom socket/networking wrapper.
- `Code/ItemEffect.h` — shared item effect constants.

**Do not** assume a change in `Basedef.cpp` only affects one server. Any struct or constant change must stay binary-compatible with the wire protocol, because both servers exchange raw message buffers.

### Architecture Notes

- **Custom binary protocol**: All client↔game and game↔DB messages are C structs with a common `_MSG` header, defined in `Basedef.h`. `#pragma pack` is used in a few places; do not change struct layouts or sizes without verifying both sides.
- **Message dispatch pattern**: `ProcessClientMessage()` (in `TMSrv/ProcessClientMessage.cpp`) is the central dispatcher. It switches on `std->Type` and calls `Exec_MSG_*()` handlers. Most handlers live in files named `_MSG_<Type>.cpp`.
- **File-based database**: `DBSrv` uses `CFileDB` to read/write account and character data from binary files on disk. There is no SQL database.
- **Monolithic, global-heavy style**: `Server.cpp` in `TMSrv` is ~9,400 lines. Extensive use of global variables and `extern` declarations is normal here.
- **Mixed Portuguese/English**: Comments, some identifiers, and commit history mix Portuguese and English.

### Toolchain Quirks

- **Character Set**: `MultiByte` (not Unicode). Do not introduce `wchar_t` APIs or UTF-16 assumptions.
- **Language Standard**: `stdcpp20` for `TMSrv`. Conformance mode is enabled (`/permissive-` equivalent via `ConformanceMode=true`), but `/Zc:strictStrings-` is explicitly passed to allow legacy string-to-char* conversions.
- **Preprocessor**: `_CRT_SECURE_NO_WARNINGS` is defined in both Debug and Release. Debug also defines `_PACKET_DEBUG`.
- **Runtime Library**: `TMSrv` Debug uses `MultiThreadedDebug` (static), but Release uses `MultiThreadedDebugDLL` (yes, DebugDLL in Release). DBSrv Release does not specify an explicit runtime, so it follows defaults.
- **Dependencies**: `Winmm.lib`, `ws2_32.lib`, plus standard Windows libs.

### Entry Points

- `TMSrv`: `Code/TMSrv/Server.cpp` — contains `WinMain`, `MainWndProc`, `InitApplication`, and the main game loop.
- `DBSrv`: `Code/DBSrv/Server.cpp` — contains the DB server window procedure, `CFileDB` integration, and account/character handling.

### Network Ports (hardcoded defaults in `Basedef.h`)

- `GAME_PORT` = 8281 — clients connect to TMSrv here.
- `DB_PORT` = 7514 — TMSrv connects to DBSrv here.
- `ADMIN_PORT` = 8895 — admin tools (NPTool) connect to DBSrv here.

### File Layout

```
W2PP Code Project.sln
Code/
  Basedef.h / Basedef.cpp       # shared core
  CPSock.h / CPSock.cpp         # shared networking
  ItemEffect.h                  # shared item constants
  TMSrv/                        # game server
    Server.cpp / Server.h
    ProcessClientMessage.cpp
    ProcessDBMessage.cpp
    ProcessSecMinTimer.cpp
    CUser.cpp / CUser.h
    CMob.cpp / CMob.h
    CItem.cpp / CItem.h
    CReadFiles.cpp / CReadFiles.h
    CCastleZakum.cpp / CCastleZakum.h
    CWarTower.cpp / CWarTower.h
    CNPCGene.cpp / CNPCGene.h
    _MSG_*.cpp                    # per-message handlers
    ...
  DBSrv/                        # database server
    Server.cpp / Server.h
    CUser.cpp / CUser.h
    CFileDB.cpp / CFileDB.h
    CReadFiles.cpp / CReadFiles.h
    CRanking.cpp / CRanking.h
    ...
  ClientPatch_v7662/            # IGNORE
```

### Important Constraints

- **Win32 only**: The code uses WinAPI (`HWND`, `HINSTANCE`, `Windows.h`) and Winsock 1/2 directly. It will not compile for x64 or non-Windows targets without significant porting.
- **No package manager**: There are no `vcpkg.json`, `conanfile.txt`, or NuGet packages. All dependencies are Windows SDK / system libraries.
- **Do not reformat large files blindly**: Many structs are carefully aligned for network serialization. Reformatting or reordering fields can break the protocol.
