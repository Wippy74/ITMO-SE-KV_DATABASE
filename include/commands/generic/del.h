#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Del : public ICommand {
public:
  std::string Name() const override {
    return "DEL";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.empty()) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    long long d = 0;
    for (const auto& k : args) {
      if (db.DelEntry(k)) {
        ++d;
      }
    }
    return MakeSize(d);
  }
};