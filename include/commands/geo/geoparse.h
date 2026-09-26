#pragma once
#include "../interface_com.h"
#include "../helpers.h"
#include "./compute_func.h"

struct GeoSearchParams {
  double cent_lon = 0;
  double cent_lat = 0;
  double radius_km = 0;
  bool ascending = true;
  long long count = -1;
};

inline bool ParseGeoSearchArgs(const std::vector<std::string>& args, size_t start, GeoSearchParams& p, std::string& err) {
  size_t i = start;
  if (i + 2 >= args.size() || ToUpper(args[i]) != "FROMLONLAT") {
    err = "syntax error"; 
    return false;
  }
  ++i;
  if (!ParseDouble(args[i], p.cent_lon)) {
    err = "invalid longitude";
    return false;
  }
  ++i;
  if (!ParseDouble(args[i], p.cent_lat)) {
    err = "invalid latitude";
    return false;
   }
  ++i;
  if (i + 1 >= args.size() || ToUpper(args[i]) != "BYRADIUS") {
    err = "syntax error"; return false;
  }
  ++i;
  double radius = 0;
  if (!ParseDouble(args[i], radius)) {
    err = "invalid radius";
    return false;
  }
  ++i;
  if (i >= args.size()) {
    err = "missing unit";
    return false;
  }
  std::string unit = args[i++];
  for (auto& c : unit) {
    c = static_cast<char>(std::tolower(c));
  }
  if (!ValidUnit(unit)) {
    err = "unsupported unit";
    return false;
  }
  p.radius_km = UnitToKm(radius, unit);
  while (i < args.size()) {
    std::string tok = ToUpper(args[i]);
    if (tok == "ASC") {
      p.ascending = true;
      ++i;
    }
    else if (tok == "DESC") {
      p.ascending = false;
      ++i;
    }
    else if (tok == "COUNT") {
      ++i;
      if (i >= args.size() || !ParseNum(args[i], p.count) || p.count <= 0) {
        err = "invalid COUNT"; return false;
      }
      ++i;
    } else {
      err = "syntax error near '" + args[i] + "'";
      return false;
    }
  }
  return true;
}

inline std::vector<std::string> RunGeoSearch(const std::unordered_map<std::string, GeoType>& gmap, const GeoSearchParams& p) {
  using Pair = std::pair<double, std::string>;
  std::vector<Pair> hits;
  for (const auto& [name, pt] : gmap) {
    double d = HaversineKm(p.cent_lon, p.cent_lat, pt.longitude, pt.latitude);
    if (d <= p.radius_km) {
      hits.push_back({d, name});
    }
  }
  auto cmp = p.ascending
    ? [](const Pair& a, const Pair& b) { return a.first < b.first; }
    : [](const Pair& a, const Pair& b) { return a.first > b.first; };
  std::sort(hits.begin(), hits.end(), cmp);
  if (p.count > 0 && static_cast<long long>(hits.size()) > p.count) {
    hits.resize(static_cast<size_t>(p.count));
  }
  std::vector<std::string> result;
  result.reserve(hits.size());
  for (auto& [_, name] : hits) {
    result.push_back(std::move(name));
  }
  return result;
}