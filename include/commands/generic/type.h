#pragma once
#include "../interface_com.h"
#include "../helpers.h"

class Type : public ICommand {
public:
  std::string Name() const override {
    return "TYPE";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 1) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      return MakeStatus("none");
    }
    return MakeStatus(std::visit(overloaded{
      [](const std::string&) {
        return "string";
      },
      [](const std::deque<std::string>&) {
        return "list";
      },
      [](const std::unordered_set<std::string>&) {
        return "set";
      },
      [](const std::unordered_map<std::string, GeoType>&) {
        return "geospacial index";
      },
    }, entry->value));
  }
};