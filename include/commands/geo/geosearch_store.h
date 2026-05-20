#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"
#include "./geoparse.h"
#include <algorithm>
#include <cctype>

class GeoSearchStore : public ICommand {
public:
  std::string Name() const override {
    return "GEOSEARCHSTORE";
  }

  OptionalResult Execute(DataBase& db, const std::vector<std::string>& args) override {
    if (args.size() < 7) {
      ErrWrongArgs(Name());
      return std::nullopt;
    }
    const std::string& dst = args[0];
    const std::string& src = args[1];
    auto* map = CheckType<std::unordered_map<std::string, GeoType>>(db, src);
    if (!map) {
      db.DelEntry(dst);
      return MakeSize(0);
    }
    GeoSearchParams p;
    std::string err;
    if (!ParseGeoSearchArgs(args, 2, p, err)) {
      std::cerr << err;
      return std::nullopt;
    }
    auto names = RunGeoSearch(*map, p);
    std::unordered_map<std::string, GeoType> result;
    for (const auto& name : names) {
      result[name] = (*map)[name];
    }
    Entry entry;
    entry.value = std::move(result);
    if (!db.SetEntry(dst, std::move(entry))) {
      return std::nullopt;
    };
    return MakeSize(names.size());
  }
};