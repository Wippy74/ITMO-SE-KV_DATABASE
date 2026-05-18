#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Exists : public ICommand {
public:
  std::string Name() const override {
    return "EXISTS";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.empty()) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    long long count = 0;
    for (const auto& key : args) {
      if (db.GetEntry(key)) {
        ++count;
      }
    }
    return MakeSize(count);
  }
};