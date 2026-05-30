#pragma once
#include "../kernel.h"
#include "../result.h"
#include <stdexcept>
#include <string>
#include <variant>
#include <iostream>
#include <cctype>

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
    size_t idx = 0;
    output = std::stoll(str, &idx);
    return idx == str.size();
  } catch (...) {
    return false;
  }
}

inline bool ParseDouble(const std::string& str, double& output) {
  try {
    size_t idx = 0;
    output = std::stod(str, &idx);
    return idx == str.size();
  } catch (...) {
    return false;
  }
}

inline void ErrWrongArgs(const std::string& cmd) {
  std::cerr << "wrong number of arguments for '" + cmd + "'\n";
}

inline void ErrWrongType() {
  std::cerr << "WRONGTYPE Operation against a key holding the wrong kind of value\n";
}

inline void ErrOutOfRange() {
  std::cerr << "value is not an integer or out of range\n";
}

inline std::string ToUpper(std::string s) {
  for (char& c : s) {
    c = static_cast<char>(std::toupper(c));
  }
  return s;
}

inline std::string ToLower(std::string s) {
  for (char& c : s) {
    c = static_cast<char>(std::tolower(c));
  }
  return s;
}

inline void DelEmpty(DataBase& db, const std::string& key) {
  Entry* entry = db.GetEntry(key);
  if (!entry) {
    return;
  }
  bool empty = std::visit(overloaded{
    [](const std::deque<std::string>& l) {
      return l.empty();
    },
    [](const std::unordered_set<std::string>& s) {
      return s.empty();
    },
    [](const auto&) {
      return false;
    },
  }, entry->value);
  if (empty) {
    db.DelEntry(key);
  }
}

inline void PrintResult(const ReturnResult& result) {
  std::visit(overloaded{
    [](ExitSignal) {},
    [](NilResult) {
      std::cout << "(nil)\n";
    },
    [](size_t n) {
      std::cout << "(integer) " << static_cast<long long>(n) << '\n';
    },
    [](const SimpleString& s) {
      std::cout << s.value << '\n';
    },
    [](const ResString& s) {
      std::cout << '"' << s.value << '"' << '\n';
    },
    [](const std::vector<std::string>& v) {
      if (v.empty()) {
        std::cout << "(empty array)\n";
        return;
      }
      for (size_t i = 0; i < v.size(); ++i) {
        std::cout << (i + 1) << ") \"" << v[i] << "\"\n";
      }
    },
  }, result);
}