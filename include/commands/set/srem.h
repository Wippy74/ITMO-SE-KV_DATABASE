#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class SRem : public ICommand {
public:
  std::string Name() const override {
    return "SREM";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* set = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (!set) {
      return MakeSize(0);
    }
    long long removed = 0;
    for (size_t i = 1; i < args.size(); ++i) {
      if (set->erase(args[i])) {
        ++removed;
      }
    }
    DelEmpty(db, args[0]);
    db.RecountEntry(args[0]);
    return MakeSize(removed);
  }
};