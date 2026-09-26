#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class LIndex : public ICommand {
public:
  std::string Name() const override {
    return "LINDEX";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    long long idx = 0;
    if (!ParseNum(args[1], idx)) {
      ErrOutOfRange();
      return std::nullopt;
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    if (!list) {
      return MakeNil();
    }
    long long len = static_cast<long long>(list->size());
    if (idx < 0) {
      idx += len;
    }
    if (idx < 0 || idx >= len) {
      return MakeNil();
    }
    return MakeString((*list)[idx]);
  }
};