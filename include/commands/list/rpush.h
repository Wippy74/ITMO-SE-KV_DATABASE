#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class RPush : public ICommand {
public:
  std::string Name() const override {
    return "RPUSH";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      Entry new_entry;
      new_entry.value = std::deque<std::string>{};
      if (!db.SetEntry(args[0], std::move(new_entry))) {
        return std::nullopt;
      };
      entry = db.GetEntry(args[0]);
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    if (!list) {
      ErrWrongType();
      return std::nullopt;
    }
    std::size_t extra = 0;
    for (std::size_t i = 1; i < args.size(); ++i) {
      extra += args[i].size() + sizeof(std::string);
    }
    if (!db.RequireMemory(extra)) {
      return std::nullopt;
    };
    for (std::size_t i = 1; i < args.size(); ++i) {
      list->push_back(args[i]);
    }
    db.RecountEntry(args[0]);
    return MakeSize(list->size());
  }
};