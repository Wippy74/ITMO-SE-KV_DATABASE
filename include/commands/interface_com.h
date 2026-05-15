#pragma once
#include <string>
#include <optional>
#include "./kernel.h"
#include "./result.h"

class ICommand {
public:
  virtual ~ICommand() = default;
  virtual OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) = 0;
  virtual std::string Name() const = 0;
};