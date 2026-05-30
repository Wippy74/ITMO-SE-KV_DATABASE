#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"
#include <iomanip>
#include <sstream>

class GeoPos : public ICommand {
public:
  std::string Name() const override {
    return "GEOPOS";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 2) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* gmap = CheckType<std::unordered_map<std::string, GeoType>>(db, args[0]);
    std::string out;
    for (size_t i = 1; i < args.size(); ++i) {
      if (i > 1) {
        out += '\n';
      }
      out += std::to_string(i) + ") ";
      if (!gmap) {
        out += "(nil)";
        continue;
      }
      auto it = gmap->find(args[i]);
      if (it == gmap->end()) {
        out += "(nil)";
      } else {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "1) \"" << it->second.longitude << "\"\n" << "   2) \"" << it->second.latitude << "\"";
        out += ss.str();
      }
    }
    return MakeStatus(out);
  }
};