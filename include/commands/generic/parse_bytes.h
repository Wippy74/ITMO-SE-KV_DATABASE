#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include <cctype>

inline bool ParseBytes(const std::string& s, size_t& out) {
  if (s.empty()) {
    return false;
  }
  size_t idx = 0;
  long long num = 0;
  try {
    num = std::stoll(s, &idx);
  } catch (...) {
    return false;
  }
  if (num < 0) {
    return false;
  }
  std::string suf = s.substr(idx);
  for (auto& c : suf) {
    c = static_cast<char>(std::tolower(c));
  }
  size_t mult = 1;
  if (suf.empty() || suf == "b") {
    mult = 1;
  }
  else if (suf == "kb") {
    mult = 1024;
  }
  else if (suf == "mb") {
    mult = 1024 * 1024;
  }
  else if (suf == "gb") {
    mult = 1024 * 1024 * 1024;
  }
  else {
    return false;
  }
  out = static_cast<size_t>(num) * mult;
  return true;
}