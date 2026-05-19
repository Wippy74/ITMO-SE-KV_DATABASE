#pragma once
#include "../interface_com.h"

class Exit : public ICommand {
public:
  std::string Name() const override {
    return "EXIT";
  }
  
  OptionalResult Execute(DataBase&, const std::vector<std::string>&) override {
    return ExitSignal{};
  }
};