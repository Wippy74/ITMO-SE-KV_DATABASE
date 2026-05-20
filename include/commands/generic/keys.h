#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./pattern_match.h"

class Keys : public ICommand {
public:
  std::string Name() const override {
    return "KEYS";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    std::vector<std::string> matched;
    for (const auto& k : db.LiveKeys()) {
      if (PatternMatch(args[0], k)) {
        matched.push_back(k);
      }
    }
    std::sort(matched.begin(), matched.end());
    return MakeArray(std::move(matched));
  }
};