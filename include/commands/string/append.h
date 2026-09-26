#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Append : public ICommand {
public:
  std::string Name() const override {
    return "APPEND";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      Entry new_entry;
      new_entry.value = args[1];
      if (!db.SetEntry(args[0], std::move(new_entry))) {
        return std::nullopt;
      };
      return MakeSize(args[1].size());
    }
    auto* str = CheckType<std::string>(db, args[0]);
    if (!str) {
      ErrWrongType();
      return std::nullopt;
    }
    if (!db.RequireMemory(args[1].size())) {
      return std::nullopt;
    };
    *str += args[1];
    db.RecountEntry(args[0]);
    return MakeSize(str->size());
  }
};