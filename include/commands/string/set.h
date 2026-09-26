#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Set : public ICommand {
public:
  std::string Name() const override {
    return "SET";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry entry;
    entry.value = args[1];
    if (!db.SetEntry(args[0], std::move(entry))) {
      return std::nullopt;
    };
    return MakeOk();
  }
};
