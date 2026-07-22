#include "DumpStruct.h"
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

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

static const char* toString(PacketDirection dir) {
  switch (dir) {
    case SERVER2CLIENT:
      return "Server2Client";
    case CLIENT2SERVER:
      return "Client2Server";
    default:
      return "Unknown";
  }
}

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

// --- repo-root helpers ---
static std::string FindRepoRoot(const char* startPath) {
  if (!startPath || !*startPath) return "";
  std::string path = startPath;
  for (;;) {
    size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) break;
    path = path.substr(0, pos);
    if (path.empty()) break;
    std::string headPath = path + "\\.git\\HEAD";
    FILE* f = fopen(headPath.c_str(), "r");
    if (f) {
      fclose(f);
      return path;
    }
  }
  return "";
}

static std::string ReadGitRevision(const char* repoRoot) {
  if (!repoRoot || !*repoRoot) return "unknown";
  std::string headPath = std::string(repoRoot) + "\\.git\\HEAD";
  FILE* f = fopen(headPath.c_str(), "r");
  if (!f) return "unknown";
  char buf[256] = {0};
  if (!fgets(buf, sizeof(buf), f)) { fclose(f); return "unknown"; }
  fclose(f);
  size_t len = strlen(buf);
  if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

  if (strncmp(buf, "ref:", 4) == 0) {
    const char* ref = buf + 4;
    while (*ref == ' ' || *ref == '\t') ++ref;
    std::string refPath = std::string(repoRoot) + "\\.git\\" + ref;
    FILE* rf = fopen(refPath.c_str(), "r");
    if (!rf) return "unknown";
    char hash[64] = {0};
    if (fgets(hash, sizeof(hash), rf)) {
      fclose(rf);
      size_t hlen = strlen(hash);
      if (hlen > 0 && hash[hlen - 1] == '\n') hash[hlen - 1] = '\0';
      return hash;
    }
    fclose(rf);
  }
  return buf; // detached HEAD
}

static bool StartsWithIgnoreCase(const char* str, const char* prefix, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (std::tolower((unsigned char)str[i]) != std::tolower((unsigned char)prefix[i]))
      return false;
  }
  return true;
}

static std::string MakeRelativePath(const char* absPath, const char* repoRoot) {
  std::string p = absPath ? absPath : "";
  std::string r = repoRoot ? repoRoot : "";
  if (r.empty()) return p;
  if (!r.empty() && r.back() != '\\' && r.back() != '/') {
    r.push_back('\\');
  }
  size_t rlen = r.length();
  if (p.length() >= rlen && StartsWithIgnoreCase(p.c_str(), r.c_str(), rlen)) {
    p = p.substr(rlen);
  }
  for (size_t i = 0; i < p.length(); ++i) {
    if (p[i] == '\\') p[i] = '/';
  }
  return p;
}

static const std::string& GetRepoRoot() {
  static std::string root = FindRepoRoot(__FILE__);
  return root;
}

static const std::string& GetRevision() {
  static std::string rev = ReadGitRevision(GetRepoRoot().c_str());
  return rev;
}

static void WriteSourceRef(FILE* f, const std::source_location& loc) {
  const std::string& fallbackRoot = GetRepoRoot();
  const std::string& revision = GetRevision();

  // Try caller's file first, then fall back to the cached root from __FILE__
  std::string callerRoot = FindRepoRoot(loc.file_name());
  const char* repoRoot = !callerRoot.empty() ? callerRoot.c_str() : fallbackRoot.c_str();

  std::string relPath = MakeRelativePath(loc.file_name(), repoRoot);

  fputs("\"source\":{", f);
  fputs("\"repository\":\"W2PP\",", f);
  fputs("\"revision\":\"", f);
  WriteJsonEscaped(f, revision.c_str());
  fputs("\",\"path\":\"", f);
  WriteJsonEscaped(f, relPath.c_str());
  fputs("\",\"function\":\"", f);
  WriteJsonEscaped(f, loc.function_name());
  fprintf(f, "\",\"line\":%d}", loc.line());
}

void DumpPacket(const char* name, const void* packet, size_t len, PacketDirection direction,
                const char* description,
                const std::source_location loc) {
  const FixtureHeader* h = (const FixtureHeader*)packet;
  FILE* f = fopen(g_fixturePath, "ab");
  if (!f) return;
  fputs("{\"fixtureType\":\"PACKET\",\"name\":\"", f);
  WriteJsonEscaped(f, name);
  fputs("\",\"schemaVersion\":1,", f);
  WriteSourceRef(f, loc);
  fputs(",\"description\":\"", f);
  WriteJsonEscaped(f, description);
  fprintf(f,
          "\",\"meta\":{\"opcode\":%u,\"opcodeHex\":\"0x%04x\","
          "\"encrypted\":false,\"endian\":\"little\",\"direction\":\"%s\"},",
          (unsigned)h->Type, (unsigned)h->Type, toString(direction));
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
  fputs("\",\"schemaVersion\":1,", f);
  WriteSourceRef(f, loc);
  fputs(",\"description\":\"", f);
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
