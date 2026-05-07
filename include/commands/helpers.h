#pragma once
#include "../kernel.h"
#include <string>
#include <variant>

template <typename T>
T* CheckType(DataBase& db, const std::string& key) {
  Entry* entry = db.GetEntry(key);
  if (!entry) {
    return nullptr;
  }
  return std::get_if<T>(&entry->value);
}

inline bool ParseNum(const std::string& str, long long& output) {
  try {
    std::size_t idx = 0;
    output = std::stoll(str, &idx);
    return idx == str.size();
  } catch (...) {
    return false;
  }
}

inline std::string ErrWrongArgs(const std::string& cmd) {
  return "(error) wrong number of arguments for '" + cmd + "'";
}

inline std::string ErrWrongType() {
  return "(error) WRONGTYPE Operation against a key holding the wrong kind of value";
}