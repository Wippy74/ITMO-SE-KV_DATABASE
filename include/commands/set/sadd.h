#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class SAdd: public ICommand {
public:
  std::string Name() const override {
    return "SADD";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      Entry new_entry;
      new_entry.value = std::unordered_set<std::string>{};
      if (!db.SetEntry(args[0], std::move(new_entry))) {
        return std::nullopt;
      };
      entry = db.GetEntry(args[0]);
    }
    auto* set = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (!set) {
      ErrWrongType();
      return std::nullopt;
    }
    size_t extra = 0;
    for (size_t i = 1; i < args.size(); ++i) {
      if (!set->count(args[i])) {
        extra += args[i].size() + sizeof(std::string) + 32;
      }
    }
    if (!db.RequireMemory(extra)) {
      return std::nullopt;
    };
    long long added = 0;
    for (size_t i = 1; i < args.size(); ++i) {
      if (set->insert(args[i]).second) {
        ++added;
      }
    }
    db.RecountEntry(args[0]);
    return MakeSize(added);
  }
};