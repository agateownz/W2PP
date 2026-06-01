#pragma once

#include <source_location>

namespace W2 {

static const char* g_fixturePath = "fixtures.ndjson";
    
void SetFixturePath(const char* path);

void DumpStruct(const char* name, const char* structName,
                const void* data, size_t len, const char* description = "", 
                const std::source_location loc = std::source_location::current());

void DumpPacket(const char* name, const void* packet,
                size_t len, const char* description = "",
                const std::source_location loc = std::source_location::current());

} // end namespace W2