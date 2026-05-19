#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"
#include <iomanip>
#include <sstream>

class GeoAdd : public ICommand {
public:
  std::string Name() const override {
    return "GEOADD";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 4 || (args.size() - 1) % 3 != 0) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    Entry* entry = db.GetEntry(args[0]);
    if (!entry) {
      Entry new_entry;
      new_entry.value = std::unordered_map<std::string, GeoType>{};
      if (!db.SetEntry(args[0], std::move(new_entry))) {
        return std::nullopt;
      };
      entry = db.GetEntry(args[0]);
    }
    auto* gmap = CheckType<std::unordered_map<std::string, GeoType>>(db, args[0]);
    if (!gmap) {
      ErrWrongType();
      return std::nullopt;
    }
    struct Point {
      double lon;
      double lat;
      std::string name; 
    };
    std::vector<Point> points;
    points.reserve((args.size() - 1) / 3);
    size_t extra = 0;
    for (size_t i = 1; i + 2 < args.size(); i += 3) {
      double lon = 0, lat = 0;
      if (!ParseDouble(args[i], lon) || !ParseDouble(args[i+1], lat)) {
        std::cerr << "value is not a valid float";
        return std::nullopt;
      }
      if (lon < -180 || lon > 180 || lat < -85.05112878 || lat > 85.05112878) {
        std::cerr << "invalid longitude,latitude pair";
        return std::nullopt;
      }
      if (!gmap->count(args[i+2])) {
        extra += args[i+2].size() + sizeof(GeoType) + 32;
      }
      points.push_back({lon, lat, args[i+2]});
    }
    if (!db.RequireMemory(extra)) {
      return std::nullopt;
    }
    long long added = 0;
    for (const auto& p : points) {
      if (!gmap->count(p.name)) {
        ++added;
      }
      (*gmap)[p.name] = {p.lon, p.lat};
    }
    db.RecountEntry(args[0]);
    return MakeSize(added);
  }
};