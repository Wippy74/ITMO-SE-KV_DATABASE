#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class LSet : public ICommand {
public:
  std::string Name() const override {
    return "LSET";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 3) {
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
      ErrWrongType();
      return std::nullopt;
    }
    long long len = static_cast<long long>(list->size());
    if (idx < 0) {
      idx += len;
    }
    if (idx < 0 || idx >= len) {
      ErrOutOfRange();
      return std::nullopt;
    }
    long long diff = static_cast<long long>(args[2].size()) - (*list)[idx].size();
    if (diff > 0 && !db.RequireMemory(static_cast<size_t>(diff))) {
      return std::nullopt;
    }
    (*list)[idx] = args[2];
    db.RecountEntry(args[0]);
    return MakeOk();
  }
};