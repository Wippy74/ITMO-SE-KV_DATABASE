#pragma once
#include <string>
#include <vector>

struct Command {
  std::string name;
  std::vector<std::string> args;
};

Command ParseCommand(const std::string& line);