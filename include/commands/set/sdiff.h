#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include <algorithm>

class SDiff : public ICommand {
public:
  std::string Name() const override {
    return "SDIFF";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.empty()) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* first = CheckType<std::unordered_set<std::string>>(db, args[0]);
    if (!first) {
      return MakeArray({});
    }
    std::unordered_set<std::string> result(*first);
    for (size_t i = 1; i < args.size(); ++i) {
      if (auto* second = CheckType<std::unordered_set<std::string>>(db, args[i])) {
        for (const auto& e : *second) {
          result.erase(e);
        }
      }
    }
    std::vector<std::string> v(result.begin(), result.end());
    return MakeArray(std::move(v));
  }
};