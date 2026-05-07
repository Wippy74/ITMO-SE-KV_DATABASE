#include <string>
#include "./kernel.h"

class ICommand {
public:
  virtual ~ICommand() = default;
  virtual std::string Execute(DataBase& db, const std::vector<std::string>& args) = 0;
  virtual std::string Name() const = 0;
};