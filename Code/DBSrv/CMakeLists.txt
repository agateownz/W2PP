# DBSrv - Database Server
add_executable(DBSrv
    # Common sources
    ${COMMON_SOURCES}
    # DBSrv sources
    CFileDB.cpp
    CRanking.cpp
    CReadFiles.cpp
    CUser.cpp
    Server.cpp
    # Resources
    DBSrv.rc
    icon1.ico
)

target_include_directories(DBSrv PRIVATE
    ${COMMON_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(DBSrv PRIVATE ${COMMON_LIBS})

# DBSrv output directory (match MSVC layout: Build/Debug/Server/DBSrv/run/)
set_target_properties(DBSrv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Build/Debug/Server/DBSrv/run"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Build/Release/Server/DBSrv/run"
)

# DBSrv compile definitions
target_compile_definitions(DBSrv PRIVATE
    $<$<CONFIG:Debug>:_PACKET_DEBUG>
    WIN32
    _WINDOWS
)

# DBSrv compile options per configuration
target_compile_options(DBSrv PRIVATE
    $<$<CONFIG:Debug>:
        /Od           # Disable optimization
        /MTd          # Multithreaded debug CRT (static)
        /W3           # Warning level 3
    >
    $<$<CONFIG:Release>:
        /O2           # Maximize speed
        /MT           # Multithreaded CRT (static)
        /W3           # Warning level 3
        /GL           # Whole program optimization
    >
)
