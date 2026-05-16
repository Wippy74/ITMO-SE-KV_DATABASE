#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class RPop : public ICommand {
public:
  std::string Name() const override {
    return "RPOP";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.empty() || args.size() > 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    if (!list || list->empty()) {
      return args.size() == 1 ? MakeNil() : MakeArray({});
    }
    if (args.size() == 1) {
      std::string back = std::move(list->back());
      list->pop_back();
      DelEmpty(db, args[0]);
      db.RecountEntry(args[0]);
      return MakeString(back);
    }
    long long count = 0;
    if (!ParseNum(args[1], count) || count < 0) {
      ErrOutOfRange();
      return std::nullopt;
    }
    size_t take = std::min(static_cast<size_t>(count), list->size());
    std::vector<std::string> result;
    result.reserve(take);
    for (std::size_t i = 0; i < take; ++i) {
      result.push_back(std::move(list->back()));
      list->pop_back();
    }
    DelEmpty(db, args[0]);
    db.RecountEntry(args[0]);
    return MakeArray(std::move(result));
  }
};