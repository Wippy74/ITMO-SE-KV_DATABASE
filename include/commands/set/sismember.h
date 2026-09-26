#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class SIsMember: public ICommand {
public:
  std::string Name() const override {
    return "SISMEMBER";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* set = CheckType<std::unordered_set<std::string>>(db, args[0]);
    return MakeSize(set && set->count(args[1]) ? 1 : 0);
  }
};