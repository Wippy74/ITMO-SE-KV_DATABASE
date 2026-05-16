#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include <algorithm>

class LInsert: public ICommand {
public:
  std::string Name() const override {
    return "LINSERT";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 4) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    std::string dir = ToUpper(args[1]);
    if (dir != "BEFORE" && dir != "AFTER") {
      std::cerr << "syntax error";
      return std::nullopt;
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    if (!list) {
      return MakeSize(0);
    }
    auto it = std::find(list->begin(), list->end(), args[2]);
    if (it == list->end()) {
      return MakeSize(-1);
    }
    if (!db.RequireMemory(args[3].size() + sizeof(std::string))) {
      return std::nullopt;
    };
    if (dir == "AFTER") {
      ++it;
    }
    list->insert(it, args[3]);
    db.RecountEntry(args[0]);
    return MakeSize(list->size());
  }
};