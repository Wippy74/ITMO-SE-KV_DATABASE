#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Ttl : public ICommand {
public:
  std::string Name() const override {
    return "TTL";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      return MakeSize(-2);
    }
    if (!entry->expires_at) {
      return MakeSize(-1);
    }
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(*entry->expires_at - std::chrono::steady_clock::now());
    if (remaining.count() <= 0) {
      db.DelEntry(args[0]);
      return MakeSize(-2);
    }
    return MakeSize(remaining.count());
  }
};