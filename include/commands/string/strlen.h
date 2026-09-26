#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Strlen : public ICommand {
public:
  std::string Name() const override {
    return "STRLEN";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* str = CheckType<std::string>(db, args[0]);
    return MakeSize(str ? str->size() : 0);
  }
};