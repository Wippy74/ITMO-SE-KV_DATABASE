#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include <algorithm>

class SUnion : public ICommand {
public:
  std::string Name() const override {
    return "SUNION";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.empty()) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    std::unordered_set<std::string> result;
    for (const auto& key : args) {
      if (auto* second = CheckType<std::unordered_set<std::string>>(db, key)) {
        result.insert(second->begin(), second->end());
      }
    }
    std::vector<std::string> v(result.begin(), result.end());
    std::sort(v.begin(), v.end());
    return MakeArray(std::move(v));
  }
};
