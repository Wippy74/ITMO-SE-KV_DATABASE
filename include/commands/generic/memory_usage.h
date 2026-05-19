#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Memory : public ICommand {
public:
  std::string Name() const override {
    return "MEMORY";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    if (ToUpper(args[0]) != "USAGE") {
      std::cerr << "unknown command'" << args[0] <<  "'";
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[1]);
    if (!entry) {
      return MakeNil();
    }
    return MakeSize(DataBase::CountEntryMemory(args[1], *entry));
  }
};