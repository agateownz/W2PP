#include "DumpStruct.h"
#include <cstdint>
#include <cstdio>
#include <sstream>

namespace W2 {

#pragma pack(push, 1)
struct FixtureHeader {
  unsigned short Size;
  unsigned char KeyWord;
  unsigned char CheckSum;
  unsigned short Type;
  unsigned short ID;
  unsigned int ClientTick;
};
#pragma pack(pop)


// --- low-level: append a raw hex byte run ---
static void WriteHex(FILE* f, const void* data, size_t len) {
  const unsigned char* p = (const unsigned char*)data;
  for (size_t i = 0; i < len; i++) fprintf(f, "%02x", p[i]);
}

// --- JSON string escaper (handles " \ and control chars) ---
static void WriteJsonEscaped(FILE* f, const char* s) {
  for (; *s; ++s) {
    unsigned char c = (unsigned char)*s;
    switch (c) {
      case '"':
        fputs("\\\"", f);
        break;
      case '\\':
        fputs("\\\\", f);
        break;
      case '\n':
        fputs("\\n", f);
        break;
      case '\r':
        fputs("\\r", f);
        break;
      case '\t':
        fputs("\\t", f);
        break;
      default:
        if (c < 0x20)
          fprintf(f, "\\u%04x", c);
        else
          fputc(c, f);
    }
  }
}

static void WriteSourceRef(FILE* f, const std::source_location loc) {
  std::stringstream ss;
  ss << loc.file_name() << "@" << loc.function_name() << ":" << loc.line();
  WriteJsonEscaped(f, ss.str().c_str());
}

void DumpPacket(const char* name, const void* packet,
                size_t len, const char* description,
                const std::source_location loc) {
  const FixtureHeader* h = (const FixtureHeader*)packet;
  FILE* f = fopen(g_fixturePath, "ab");
  if (!f) return;
  fputs("{\"fixtureType\":\"PACKET\",\"name\":\"", f);
  WriteJsonEscaped(f, name);
  fputs("\",\"schemaVersion\":1,\"sourceRef\":\"", f);
  WriteSourceRef(f, loc);
  fputs("\",\"description\":\"", f);
  WriteJsonEscaped(f, description);
  fprintf(f,
          "\",\"meta\":{\"opcode\":%u,\"opcodeHex\":\"0x%04x\","
          "\"encrypted\":false,\"endian\":\"little\"},",
          (unsigned)h->Type, (unsigned)h->Type);
  fprintf(f,
          "\"header\":{\"size\":%u,\"keyWord\":%u,\"checkSum\":%u,"
          "\"type\":%u,\"id\":%u,\"clientTick\":%u},",
          (unsigned)h->Size, (unsigned)h->KeyWord, (unsigned)h->CheckSum,
          (unsigned)h->Type, (unsigned)h->ID, (unsigned)h->ClientTick);
  fprintf(f, "\"byteLength\":%zu,\"bytes\":\"", len);
  WriteHex(f, packet, len);
  fputs("\"}\n", f);
  fclose(f);
}
void DumpStruct(const char* name, const char* structName,
                const void* data, size_t len, const char* description,
                const std::source_location loc) {
  FILE* f = fopen(g_fixturePath, "ab");
  if (!f) return;
  fputs("{\"fixtureType\":\"STRUCT\",\"name\":\"", f);
  WriteJsonEscaped(f, name);
  fputs("\",\"schemaVersion\":1,\"sourceRef\":\"", f);
  WriteSourceRef(f, loc);
  fputs("\",\"description\":\"", f);
  WriteJsonEscaped(f, description);
  fprintf(f,
          "\",\"meta\":{\"structName\":\"%s\",\"declaredSize\":%zu,"
          "\"pragmaPack\":1,\"endian\":\"little\"},",
          structName, len);
  fprintf(f, "\"byteLength\":%zu,\"bytes\":\"", len);
  WriteHex(f, data, len);
  fputs("\"}\n", f);
  fclose(f);
}
} // end namespace W2