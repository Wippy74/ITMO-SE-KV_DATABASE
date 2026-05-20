#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"
#include "./geoparse.h"
#include <algorithm>

class GeoSearch : public ICommand {
public:
  std::string Name() const override {
    return "GEOSEARCH"; 
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 6) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* map = CheckType<std::unordered_map<std::string, GeoType>>(db, args[0]);
    if (!map) {
      return MakeArray({});
    }
    GeoSearchParams p;
    std::string err;
    if (!ParseGeoSearchArgs(args, 1, p, err)) {
      std::cerr << err;
      return std::nullopt;
    }
    return MakeArray(RunGeoSearch(*map, p));
  }
};