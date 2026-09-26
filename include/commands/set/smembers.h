#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include <algorithm>

class SMembers : public ICommand {
public:
  std::string Name() const override {
    return "SMEMBERS";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* set = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (!set) {
      return MakeArray({});
    }
    std::vector<std::string> members(set->begin(), set->end());
    return MakeArray(std::move(members));
  }
};