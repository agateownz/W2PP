cmake_minimum_required(VERSION 3.15)
project(W2PP VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common include directories
set(COMMON_INCLUDES
    "${PROJECT_SOURCE_DIR}/Code"
    "${PROJECT_SOURCE_DIR}"
)

# Common source files (shared between projects)
set(COMMON_SOURCES
    "${PROJECT_SOURCE_DIR}/Code/Basedef.cpp"
    "${PROJECT_SOURCE_DIR}/Code/CPSock.cpp"
)

set(COMMON_HEADERS
    "${PROJECT_SOURCE_DIR}/Code/Basedef.h"
    "${PROJECT_SOURCE_DIR}/Code/CPSock.h"
    "${PROJECT_SOURCE_DIR}/Code/ItemEffect.h"
)

# Common Windows libraries for server executables
set(COMMON_LIBS
    winmm.lib
    ws2_32.lib
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
)

# Suppress CRT secure warnings for all targets
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# Enable UTF-8 source file encoding for all targets
add_compile_options(/utf-8)

# Configure output directories to match MSVC layout
# (must be set before defining targets)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Build/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Build/Release")

# Default to Windows subsystem for all executables
set(CMAKE_WIN32_EXECUTABLE ON)

# Add subdirectories for each project
add_subdirectory(Code/DBSrv)
add_subdirectory(Code/TMSrv)
add_subdirectory(Code/ClientPatch_v7662)

# Convenience target to build all servers at once
add_custom_target(build_all_servers
    DEPENDS DBSrv TMSrv ClientPatch_v7662
    COMMENT "Building all server executables"
)

# Set build_all_servers as default target
set_property(DIRECTORY PROPERTY EP_DEFAULT_TARGET build_all_servers)

# Installation
include(GNUInstallDirs)
install(TARGETS DBSrv TMSrv ClientPatch_v7662
    RUNTIME DESTINATION bin
)
