# W2PP Linux Migration Plan

This document outlines the multi-step plan to migrate DBSrv and TMSrv from Windows to Linux.

## Overview

The W2PP servers (DBSrv and TMSrv) are heavily coupled with Win32 APIs including:
- GUI/Window management (WinMain, WindowProc, GDI)
- Winsock with WSAAsyncSelect (event-driven sockets)
- Windows-specific types and macros
- File operations with Windows APIs

The migration will be done in **phases** to minimize risk and allow incremental testing.

---

## Phase 1: Dependency Management with vcpkg

**Goal:** Set up cross-platform dependency management.

### Tasks

1. **Install vcpkg**
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh  # Linux
   ./bootstrap-vcpkg.bat # Windows
   ```

2. **Create vcpkg.json manifest** in project root:
   ```json
   {
     "name": "w2pp",
     "version": "1.0.0",
     "dependencies": [
       "asio",
       "spdlog",
       "fmt"
     ]
   }
   ```

3. **Update CMakeLists.txt** to use vcpkg toolchain:
   ```cmake
   # Add at the top of root CMakeLists.txt
   if(DEFINED ENV{VCPKG_ROOT})
       set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
   endif()
   ```

4. **Install dependencies on both platforms**:
   ```bash
   vcpkg install asio spdlog fmt
   ```

### Verification
- [ ] Project builds on Windows with vcpkg dependencies
- [ ] No functional changes - behavior identical to before

---

## Phase 2: Logging Modernization (spdlog)

**Goal:** Replace all logging/output with spdlog, removing GDI text output dependency.

### Current State
- `TextOutWind()` in DBSrv/Server.cpp uses GDI to draw text on window
- `MessageBox()` calls throughout codebase
- `Log()` function writes to files

### Tasks

1. **Create logging wrapper** (`Code/Common/Logger.h`):
   ```cpp
   #pragma once
   #include <spdlog/spdlog.h>
   #include <spdlog/sinks/rotating_file_sink.h>
   #include <spdlog/sinks/stdout_color_sinks.h>
   
   class Logger {
   public:
       static void Initialize(const std::string& name);
       static std::shared_ptr<spdlog::logger> Get();
       
       // For GUI compatibility during transition
       static void SetGuiCallback(std::function<void(const char*, int)> callback);
   };
   
   #define LOG_INFO(...) Logger::Get()->info(__VA_ARGS__)
   #define LOG_ERROR(...) Logger::Get()->error(__VA_ARGS__)
   #define LOG_WARN(...) Logger::Get()->warn(__VA_ARGS__)
   ```

2. **Replace GDI text output**:
   - Replace `TextOutWind()` calls with `LOG_INFO()`
   - Remove `GetDC()`, `SetTextColor()`, `TextOutA()` calls
   - Remove font handling (`HFONT`, `CreateFont`, `SelectObject`)

3. **Replace MessageBox**:
   - Create `ShowError()` function that logs to spdlog
   - Replace all `MessageBox()` calls with `LOG_ERROR()` + `ShowError()`

4. **Update file logging**:
   - Replace custom `Log()` function with spdlog file sinks
   - Use rotating file logger for day logs

### Files to Modify
- `Code/DBSrv/Server.cpp` - Remove GDI, add spdlog
- `Code/TMSrv/Server.cpp` - Remove GDI, add spdlog
- `Code/Basedef.cpp` - Replace MessageBox calls
- `Code/CPSock.cpp` - Replace MessageBox calls

### Verification
- [ ] Logs appear in console (Windows)
- [ ] Logs written to rotating files
- [ ] No GDI dependencies remaining for text output

---

## Phase 3: Socket Abstraction Layer

**Goal:** Abstract all socket operations to allow swapping Winsock with ASIO.

### Current State
- `CPSock` class uses Winsock with `WSAAsyncSelect` (Windows message-based async)
- Socket events posted to window message queue
- Heavy coupling between socket and GUI message loop

### Tasks

1. **Create socket interface** (`Code/Common/ISocket.h`):
   ```cpp
   #pragma once
   #include <functional>
   #include <string>
   
   enum class SocketEvent {
       Accept,
       Read,
       Write,
       Close,
       Error
   };
   
   class ISocket {
   public:
       virtual ~ISocket() = default;
       
       virtual bool Initialize() = 0;
       virtual bool Listen(const std::string& address, int port) = 0;
       virtual bool Connect(const std::string& host, int port) = 0;
       virtual void Close() = 0;
       
       virtual int Send(const char* data, int size) = 0;
       virtual int Receive(char* buffer, int maxSize) = 0;
       
       // Async event handling
       using EventCallback = std::function<void(SocketEvent, int errorCode)>;
       virtual void SetEventCallback(EventCallback callback) = 0;
       
       // For server accept
       virtual std::unique_ptr<ISocket> Accept() = 0;
   };
   ```

2. **Create ASIO implementation** (`Code/Common/AsioSocket.h/cpp`):
   - Implement `ISocket` using `asio::ip::tcp`
   - Support async operations with callbacks
   - No Windows message loop dependency

3. **Create compatibility wrapper** for existing CPSock:
   - Keep CPSock interface but delegate to ISocket
   - Remove `HWND` parameter from `StartListen()`
   - Remove `WSAAsyncSelect` dependency

4. **Update message pump**:
   - Replace Windows message loop with ASIO io_context run
   - Integrate with timer events

### Files to Modify
- `Code/CPSock.h/cpp` - Refactor to use ISocket
- `Code/DBSrv/Server.cpp` - Replace message loop
- `Code/TMSrv/Server.cpp` - Replace message loop
- `Code/DBSrv/CUser.cpp` - Update socket handling
- `Code/TMSrv/CUser.cpp` - Update socket handling

### Verification
- [ ] DBSrv accepts connections via ASIO
- [ ] TMSrv accepts connections via ASIO
- [ ] No WSAAsyncSelect calls remaining
- [ ] No HWND dependencies in socket code

---

## Phase 4: Cross-Platform Types and Utilities

**Goal:** Eliminate Windows-specific types and utility functions.

### Tasks

1. **Create platform abstraction header** (`Code/Common/Platform.h`):
   ```cpp
   #pragma once
   
   #ifdef _WIN32
       #include <windows.h>
       #include <winsock2.h>
   #else
       #include <sys/types.h>
       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>
       #include <unistd.h>
       #include <dirent.h>
       #include <sys/time.h>
       
       // Windows type mappings
       typedef int SOCKET;
       #define INVALID_SOCKET (-1)
       #define SOCKET_ERROR (-1)
       
       typedef unsigned char BYTE;
       typedef unsigned short WORD;
       typedef unsigned int DWORD;
       typedef int BOOL;
       #define TRUE 1
       #define FALSE 0
       
       // Byte manipulation macros
       #define LOWORD(l) ((WORD)(l))
       #define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
       #define LOBYTE(w) ((BYTE)(w))
       #define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
       #define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
       
       // Socket functions
       #define closesocket close
       #define Sleep(ms) usleep((ms) * 1000)
   #endif
   ```

2. **Replace timing functions**:
   - Create `GetTickCount()` wrapper using `std::chrono`
   - Create `timeGetTime()` wrapper
   - Replace all timing calls

3. **Replace file operations**:
   - Create `FindFirstFile`/`FindNextFile` wrapper using `dirent.h`
   - Replace `SetCurrentDirectory` with `chdir`
   - Replace Windows file time with `stat`/`std::filesystem`

4. **Replace string functions**:
   - Replace `stricmp` with `strcasecmp`
   - Replace `CharNext` with standard iteration

---

## Phase 4b: Modernize File I/O with C++ Standard Library

**Goal:** Replace C-style file operations (`_open`, `fopen`, `fread`, `fwrite`) with C++ standard streams (`ifstream`, `ofstream`, `fstream`).

### Current State
- File operations use C-style functions: `_open`, `_read`, `_close`, `fopen`, `fread`, `fwrite`, `fclose`, `fseek`, `ftell`
- Windows-specific flags: `_O_RDONLY`, `_O_BINARY`, `_O_CREAT`
- Manual buffer management for binary file operations
- No RAII - files must be explicitly closed

### Target State
- Use `std::ifstream` for reading
- Use `std::ofstream` for writing
- Use `std::fstream` for read/write
- Use `std::filesystem` for path operations
- RAII ensures files are automatically closed
- Type-safe operations with less manual buffer management

### Tasks

1. **Create file utility wrapper** (`Code/Common/FileUtils.h/cpp`):
   ```cpp
   #pragma once
   #include <fstream>
   #include <vector>
   #include <string>
   #include <filesystem>

   namespace FileUtils {
       // Read entire file into vector
       std::vector<unsigned char> ReadFile(const std::filesystem::path& path);
       
       // Read entire file into string
       std::string ReadTextFile(const std::filesystem::path& path);
       
       // Write vector to file
       bool WriteFile(const std::filesystem::path& path, const std::vector<unsigned char>& data);
       
       // Write string to file
       bool WriteTextFile(const std::filesystem::path& path, const std::string& data);
       
       // Check if file exists
       bool Exists(const std::filesystem::path& path);
       
       // Get file size
       size_t GetSize(const std::filesystem::path& path);
       
       // Create directory (recursive)
       bool CreateDirectory(const std::filesystem::path& path);
       
       // Get current working directory
       std::filesystem::path GetCurrentDirectory();
       
       // Set current working directory
       bool SetCurrentDirectory(const std::filesystem::path& path);
       
       // List files in directory (replacement for FindFirstFile/FindNextFile)
       std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& directory);
       
       // Binary file reader with position tracking
       class BinaryReader {
       private:
           std::ifstream m_stream;
           std::streampos m_position;
       public:
           explicit BinaryReader(const std::filesystem::path& path);
           ~BinaryReader();
           
           bool IsOpen() const;
           void Seek(std::streampos position);
           std::streampos Tell() const;
           size_t GetSize() const;
           
           template<typename T>
           bool Read(T& value);
           bool ReadBytes(void* buffer, size_t size);
           std::vector<unsigned char> ReadAll();
       };
       
       // Binary file writer
       class BinaryWriter {
       private:
           std::ofstream m_stream;
       public:
           explicit BinaryWriter(const std::filesystem::path& path, bool append = false);
           ~BinaryWriter();
           
           bool IsOpen() const;
           void Flush();
           
           template<typename T>
           bool Write(const T& value);
           bool WriteBytes(const void* buffer, size_t size);
       };
   }
   ```

2. **Migrate database file operations**:
   - `CFileDB.cpp` - Account/character database files
   - `CReadFiles.cpp` - Game data files (NPCs, items, etc.)
   - Replace `fopen`/`fread` with `FileUtils::BinaryReader`
   - Replace `fwrite` with `FileUtils::BinaryWriter`

3. **Migrate log file operations**:
   - Replace `fopen` for log files with `std::ofstream`
   - Use `spdlog` for structured logging (already done in Phase 2)

4. **Migrate ranking file operations**:
   - `CRanking.cpp` - Ranking data files
   - Use `FileUtils` wrappers

5. **Update path handling**:
   - Replace string concatenation with `std::filesystem::path`
   - Use `std::filesystem::path::operator/` for path joining
   - Example: `path / "subdir" / "file.txt"`

### Example Migration

**Before (C-style):**
```cpp
// Reading a file
FILE* fp = fopen(filename, "rb");
if (fp == NULL)
    return FALSE;

fseek(fp, 0, SEEK_END);
int size = ftell(fp);
fseek(fp, 0, SEEK_SET);

char* buffer = (char*)malloc(size);
fread(buffer, 1, size, fp);
fclose(fp);

// Writing a file
FILE* fp = fopen(filename, "wb");
if (fp) {
    fwrite(data, 1, size, fp);
    fclose(fp);
}
```

**After (C++ streams):**
```cpp
// Reading a file
auto buffer = FileUtils::ReadFile(filename);
if (buffer.empty())
    return false;

// Or using BinaryReader
FileUtils::BinaryReader reader(filename);
if (!reader.IsOpen())
    return false;
auto buffer = reader.ReadAll();

// Writing a file
FileUtils::WriteFile(filename, data);

// Or using BinaryWriter
FileUtils::BinaryWriter writer(filename);
writer.WriteBytes(data.data(), data.size());
```

### Files to Modify
- `Code/DBSrv/CFileDB.cpp` - Account/character database
- `Code/DBSrv/CReadFiles.cpp` - Game data loading
- `Code/DBSrv/CRanking.cpp` - Ranking files
- `Code/TMSrv/CReadFiles.cpp` - Game data loading (TMSrv)
- `Code/Basedef.cpp` - Any file operations

### Verification
- [ ] All file operations use C++ streams
- [ ] No `_open`, `_read`, `_close` calls remaining
- [ ] No `fopen`, `fread`, `fwrite`, `fclose` calls remaining
- [ ] Binary file compatibility maintained (same file format)
- [ ] Performance comparable or better than C-style
- [ ] Files are automatically closed (RAII)
- [ ] Path handling uses `std::filesystem`

### Benefits
- **Type safety**: Templates prevent type mismatches
- **RAII**: Files automatically closed on scope exit
- **Exception safety**: Destructors called during exceptions
- **Cross-platform**: `std::filesystem` handles path separators
- **Modern C++**: Consistent with rest of modernized codebase
- **Easier to maintain**: Higher-level abstractions

### Files to Modify
- `Code/Basedef.h/cpp` - Add Platform.h, replace types
- `Code/DBSrv/CFileDB.cpp` - Replace file operations
- `Code/DBSrv/CRanking.cpp` - Replace timing and file operations
- `Code/TMSrv/ProcessSecMinTimer.cpp` - Replace timing

### Verification
- [ ] Compiles on Windows with new abstractions
- [ ] All Windows types defined in Platform.h
- [ ] File operations work correctly
- [ ] Timing functions work correctly

---

## Phase 5: Remove GUI Dependencies

**Goal:** Convert from GUI application to console application.

### Current State
- WinMain entry point
- Window class registration
- Message loop (GetMessage/DispatchMessage)
- Menu system
- GDI drawing for status display

### Tasks

1. **Create console application entry point**:
   - Add `main()` function alongside `WinMain()`
   - Use `#ifdef _WIN32` to select entry point
   - Or create separate console main file

2. **Remove window creation**:
   - Remove `RegisterClass`, `CreateWindow`, `ShowWindow`
   - Remove `LoadIcon`, `LoadCursor`
   - Remove menu creation (`CreateMenu`, `AppendMenu`)

3. **Replace message loop**:
   - Create `RunEventLoop()` function
   - On Windows: can still pump messages if needed
   - On Linux: pure ASIO io_context run

4. **Handle signals**:
   - Add SIGINT/SIGTERM handlers for graceful shutdown
   - Replace WM_CLOSE handling

5. **Command-line interface**:
   - Add CLI arguments for configuration
   - Replace registry/config file reading

### Files to Modify
- `Code/DBSrv/Server.cpp` - Remove GUI, add console
- `Code/TMSrv/Server.cpp` - Remove GUI, add console

### Verification
- [ ] Runs as console application on Windows
- [ ] No window created
- [ ] Responds to Ctrl+C
- [ ] Logs to console and file

---

## Phase 6: Linux Build System

**Goal:** Build and run on Linux with clang/gcc.

### Tasks

1. **Update CMakeLists.txt for Linux**:
   ```cmake
   if(WIN32)
       add_executable(DBSrv WIN32 ...)  # Windows GUI
   else()
       add_executable(DBSrv ...)        # Linux console
       target_link_libraries(DBSrv pthread)
   endif()
   ```

2. **Handle platform-specific code**:
   - Use `#ifdef _WIN32` for Windows-only code
   - Use `#ifdef __linux__` for Linux-specific code
   - Keep platform-specific sections minimal

3. **Fix Linux-specific issues**:
   - Case-sensitive filenames
   - Path separators (`/` vs `\`)
   - Line endings (CRLF vs LF)

4. **Create Linux build scripts**:
   ```bash
   #!/bin/bash
   mkdir -p build-linux
   cd build-linux
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
   make -j$(nproc)
   ```

### Verification
- [ ] Compiles on Linux with clang
- [ ] Compiles on Linux with gcc
- [ ] All tests pass
- [ ] No compiler warnings

---

## Phase 7: Testing and Validation

**Goal:** Ensure feature parity between Windows and Linux versions.

### Tasks

1. **Unit tests**:
   - Create tests for socket abstraction
   - Create tests for file operations
   - Create tests for protocol handling

2. **Integration tests**:
   - Test DBSrv <-> TMSrv communication
   - Test client connection
   - Test database operations

3. **Performance testing**:
   - Compare Windows vs Linux performance
   - Profile ASIO vs WSAAsyncSelect
   - Memory usage comparison

4. **Stress testing**:
   - High connection count
   - Long-running stability
   - Resource leak detection

### Verification
- [ ] All unit tests pass
- [ ] Integration tests pass
- [ ] Performance acceptable
- [ ] No memory leaks

---

## Phase 8: Deployment and Documentation

**Goal:** Production-ready Linux deployment.

### Tasks

1. **Create systemd service files**:
   ```ini
   # /etc/systemd/system/w2pp-dbsrv.service
   [Unit]
   Description=W2PP Database Server
   After=network.target
   
   [Service]
   Type=simple
   User=w2pp
   WorkingDirectory=/opt/w2pp
   ExecStart=/opt/w2pp/DBSrv
   Restart=always
   
   [Install]
   WantedBy=multi-user.target
   ```

2. **Create Docker container**:
   ```dockerfile
   FROM ubuntu:22.04
   RUN apt-get update && apt-get install -y libasio-dev
   COPY build-linux/DBSrv /usr/local/bin/
   COPY build-linux/TMSrv /usr/local/bin/
   EXPOSE 8080 8081
   CMD ["DBSrv"]
   ```

3. **Update documentation**:
   - Linux build instructions
   - Deployment guide
   - Configuration reference
   - Troubleshooting guide

### Verification
- [ ] Systemd service works
- [ ] Docker container runs
- [ ] Documentation complete

---

## Migration Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: vcpkg | 1-2 days | None |
| Phase 2: Logging | 3-5 days | Phase 1 |
| Phase 3: Sockets | 7-10 days | Phase 1, 2 |
| Phase 4: Types/Utils | 5-7 days | None |
| Phase 5: GUI Removal | 5-7 days | Phase 2, 3, 4 |
| Phase 6: Linux Build | 3-5 days | Phase 1-5 |
| Phase 7: Testing | 5-7 days | Phase 6 |
| Phase 8: Deployment | 2-3 days | Phase 7 |
| **Total** | **~30-45 days** | |

---

## Risk Mitigation

### High Risk Areas

1. **WSAAsyncSelect Replacement**
   - Risk: Event handling timing differences
   - Mitigation: Extensive testing, keep Windows version working

2. **GUI to Console Conversion**
   - Risk: Loss of visual feedback for operators
   - Mitigation: Add web dashboard or ncurses UI later

3. **File Path Handling**
   - Risk: Path separator issues
   - Mitigation: Use `std::filesystem` throughout

4. **Character Encoding**
   - Risk: Portuguese characters in source files
   - Mitigation: Ensure UTF-8 everywhere

### Rollback Strategy

- Keep Windows version functional throughout migration
- Use feature branches for each phase
- Maintain compatibility layer until Phase 6 complete

---

## Recommended Libraries

| Purpose | Library | Rationale |
|---------|---------|-----------|
| Networking | ASIO | Cross-platform, async, proven |
| Logging | spdlog | Fast, header-only, widely used |
| String formatting | fmt | Modern, type-safe, fast |
| CLI arguments | CLI11 | Modern C++, easy to use |
| Configuration | toml11 | Human-readable config files |
| Testing | Catch2 | Modern, header-only |

---

## Success Criteria

1. **Functional**: All features work on Linux as on Windows
2. **Performance**: No more than 10% performance regression
3. **Stability**: Can run 24/7 without crashes or memory leaks
4. **Maintainability**: Code is cleaner and better organized
5. **Portability**: Can build on any modern Linux distribution
