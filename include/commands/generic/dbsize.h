#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class DBSize : public ICommand {
public:
  std::string Name() const override {
    return "DBSIZE";
  }
  OptionalResult Execute(DataBase& db, const std::vector<std::string>&) override {
    return MakeSize(db.Size());
  }
};