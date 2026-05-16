#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class LLen : public ICommand {
public:
  std::string Name() const override {
    return "LLEN";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    return MakeSize(list ? list->size() : 0);
  }
};