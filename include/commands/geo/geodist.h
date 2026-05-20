#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"
#include <iomanip>
#include <sstream>

class GeoDist : public ICommand {
public:
  std::string Name() const override {
    return "GEODIST";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 3) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    auto* gmap = CheckType<std::unordered_map<std::string, GeoType>>(db, args[0]);
    if (!gmap) {
      return MakeNil();
    }
    auto it1 = gmap->find(args[1]);
    auto it2 = gmap->find(args[2]);
    if (it1 == gmap->end() || it2 == gmap->end()) {
      return MakeNil();
    }
    std::string unit = "m";
    if (args.size() >= 4) {
      unit = ToLower(args[3]);
      if (!ValidUnit(unit)) {
        std::cerr << "unknown unit";
        return std::nullopt;
      }
    }
    double distKm = HaversineKm(it1->second.longitude, it1->second.latitude, it2->second.longitude, it2->second.latitude);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << KmToUnit(distKm, unit);
    return MakeString(ss.str());
  }
};