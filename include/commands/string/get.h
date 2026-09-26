#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Get : public ICommand {
public:
  std::string Name() const override { return "GET"; }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* str = CheckType<std::string>(db, args[0]);
    if (!str) {
      return MakeNil();
    }
    return MakeString(*str);
  }
};