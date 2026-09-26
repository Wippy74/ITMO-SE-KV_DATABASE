#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class FlushDB : public ICommand {
public:
  std::string Name() const override {
    return "FLUSHDB";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>&) override {
    db.FlushDB();
    return MakeOk();
  }
};