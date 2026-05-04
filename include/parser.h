#pragma once
#include <string>
#include <vector>

struct Commands {
  std::string name;
  std::vector<std::string> args;
};

Commands ParseCommand(const std::string& line);