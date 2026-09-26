#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "./commands/interface_com.h"
#include "./parser.h"
#include "./kernel.h"

class CommandsDispatcher {
public:
  CommandsDispatcher();
  OptionalResult Dispatch(DataBase& db, const Command& cmd);
private:
  void Register(std::unique_ptr<ICommand> cmd);
  std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
};