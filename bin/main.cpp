#include "../include/kernel.h"
#include "../include/parser.h"
#include "../include/dispatcher.h"
#include "../include/commands/generic/parse_bytes.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  DataBase db;
  CommandsDispatcher dispatcher;
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--maxmemory" && i + 1 < argc) {
      std::size_t bytes = 0;
      if (!ParseBytes(argv[++i], bytes)) {
        std::cerr << "Invalid --maxmemory value\n";
        return 1;
      }
      db.Memory().SetMax(bytes);
    }
  }
  std::string line;
  while (std::getline(std::cin, line)) {
    Command cmd = ParseCommand(line);
    if (cmd.name.empty()) {
      continue;
    }
    OptionalResult result = dispatcher.Dispatch(db, cmd);
    if (!result) {
      continue;
    }
    if (std::holds_alternative<ExitSignal>(*result)) {
      break;
    }
    PrintResult(*result);
  }
  return 0;
}