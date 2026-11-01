// Minimal Arduino shim so model.cpp / ui.cpp build on a desktop.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <initializer_list>
using std::max; using std::min;
uint32_t millis();                       // provided by sim_main.cpp (SDL ticks)
inline void delayMicroseconds(int) {}
#define constrain(x,a,b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
class String {
  std::string s;
 public:
  String(const char* c = "") : s(c) {}
  size_t length() const { return s.size(); }
  const char* c_str() const { return s.c_str(); }
};
