# ClientPatch_v7662 - Client Patcher DLL
add_library(ClientPatch_v7662 SHARED
    main.cpp
)

target_include_directories(ClientPatch_v7662 PRIVATE
    ${COMMON_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(ClientPatch_v7662 PRIVATE ${COMMON_LIBS})

# ClientPatch output directory (match MSVC layout: Build/Debug/Client/)
set_target_properties(ClientPatch_v7662 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Build/Client"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Build/Client"
    # Debug DLL gets _d suffix to match original MSVC build
    OUTPUT_NAME_DEBUG "ClientPatch_v7662_d"
)

# ClientPatch compile options per configuration
target_compile_options(ClientPatch_v7662 PRIVATE
    $<$<CONFIG:Debug>:
        /Od
        /W3
    >
    $<$<CONFIG:Release>:
        /O1
        /W3
        /GL
    >
)
