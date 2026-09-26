#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./parse_bytes.h"

class Config : public ICommand {
public:
  std::string Name() const override {
    return "CONFIG";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    std::string sub = ToUpper(args[0]);
    std::string param = ToLower(args[1]);
    if (param != "maxmemory") {
      std::cerr << "unsupported command: " << args[1];
      return std::nullopt;
    }
    if (sub == "GET") {
      return MakeArray({"maxmemory", std::to_string(db.Memory().GetMax())});
    }
    if (sub == "SET") {
      if (args.size() < 3) {
        ErrWrongArgs(Name());
        return std::nullopt;
      }
      size_t bytes = 0;
      if (!ParseBytes(args[2], bytes)) {
        std::cerr << "invalid maxmemory value";
        return std::nullopt;
      }
      db.Memory().SetMax(bytes);
      return MakeOk();
    }
    std::cerr << "unknown CONFIG subcommand '" + args[0];
    return std::nullopt;
  }
};