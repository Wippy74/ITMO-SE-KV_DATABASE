#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class SMove : public ICommand {
public:
  std::string Name() const override {
    return "SMOVE";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 3) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* src = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (!src || !src->count(args[2])) {
      return MakeSize(0);
    }
    Entry* dstEntry = db.GetEntry(args[1]);
    if (!dstEntry) {
      Entry new_entry; 
      new_entry.value = std::unordered_set<std::string>{};
      if (!db.SetEntry(args[1], std::move(new_entry))) {
        return std::nullopt;
      };
      dstEntry = db.GetEntry(args[1]);
    }
    auto* dst = CheckType<std::unordered_set<std::string>>(db, args[1]);
    if (!dst) {
      ErrWrongType();
      return std::nullopt;
    }
    if (!dst->count(args[2]) && !db.RequireMemory(args[2].size() + sizeof(std::string) + 32)) {
      return std::nullopt;
    }
    dst->insert(args[2]);
    src = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (src) {
      src->erase(args[2]);
      DelEmpty(db, args[0]);
      db.RecountEntry(args[0]);
    }
    db.RecountEntry(args[1]);
    return MakeSize(1);
  }
};