#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Expire : public ICommand {
public:
  std::string Name() const override {
    return "EXPIRE";
  
  }
  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    long long seconds = 0;
    if (!ParseNum(args[1], seconds) || seconds <= 0) {
      ErrOutOfRange();
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      return MakeSize(0);
    }
    entry->expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    return MakeSize(1);
  }
};