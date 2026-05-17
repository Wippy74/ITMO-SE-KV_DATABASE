#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class LRange : public ICommand {
public:
  std::string Name() const override {
    return "LRANGE";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 3) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    long long start_raw = 0;
    long long stop_raw = 0;
    if (!ParseNum(args[1], start_raw) || !ParseNum(args[2], stop_raw)) {
      ErrOutOfRange();
      return std::nullopt;
    }
    auto* list = CheckType<std::deque<std::string>>(db, args[0]);
    if (!list || list->empty()) {
      return MakeArray({});
    }
    long long len = static_cast<long long>(list->size());
    long long start = start_raw < 0 ? start_raw + len : start_raw;
    long long stop  = stop_raw  < 0 ? stop_raw  + len : stop_raw;
    if (start < 0) {
      start = 0;
    }
    if (stop >= len) {
      stop = len - 1;
    }
    if (start > stop) {
      return MakeArray({});
    }
    std::vector<std::string> result;
    result.reserve(static_cast<size_t>(stop - start + 1));
    for (long long i = start; i <= stop; ++i) {
      result.push_back((*list)[static_cast<size_t>(i)]);
    }
    return MakeArray(std::move(result));
  }
};