#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./pattern_match.h"

inline bool Impl(const std::string& pattern, size_t pattern_it, const std::string& key, size_t key_it) {
  while (pattern_it < pattern.size()) {
    if (pattern[pattern_it] == '*') {
      ++pattern_it;
      for (size_t k = key_it; k <= key.size(); ++k) {
        if (Impl(pattern, pattern_it, key, k)) {
          return true;
        }
      }
      return false;
    }
    if (key_it >= key.size()) {
      return false;
    }
    if (pattern[pattern_it] != '?' && pattern[pattern_it] != key[key_it]) {
      return false;
    }
    ++pattern_it;
    ++key_it;
  }
  return key_it == key.size();
}

inline bool PatternMatch(const std::string& pattern, const std::string& key) {
  return Impl(pattern, 0, key, 0);
}