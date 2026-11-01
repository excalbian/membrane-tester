// In-memory Preferences (NVS) shim; sim_main.cpp saves it to a file on exit.
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Arduino.h"
extern std::map<std::string, std::vector<uint8_t>> sim_nvs;
class Preferences {
 public:
  bool begin(const char*, bool) { return true; }
  void end() {}
  uint8_t getUChar(const char* k, uint8_t d) { auto it = sim_nvs.find(k); return it == sim_nvs.end() ? d : it->second[0]; }
  void putUChar(const char* k, uint8_t v) { sim_nvs[k] = {v}; }
  size_t getBytes(const char* k, void* b, size_t n) { auto it = sim_nvs.find(k); if (it == sim_nvs.end()) return 0; size_t m = std::min(n, it->second.size()); memcpy(b, it->second.data(), m); return m; }
  size_t putBytes(const char* k, const void* b, size_t n) { sim_nvs[k] = std::vector<uint8_t>((const uint8_t*)b, (const uint8_t*)b + n); return n; }
  bool remove(const char* k) { return sim_nvs.erase(k); }
  void putString(const char* k, const char* v) { putBytes(k, v, strlen(v) + 1); }
  String getString(const char* k, const char* d) { auto it = sim_nvs.find(k); return it == sim_nvs.end() ? String(d) : String((const char*)it->second.data()); }
};
