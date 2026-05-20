#pragma once
#include <cmath>
#include <string>

constexpr double kRad = 6372.8;

inline double HaversineKm(double lon1, double lat1, double lon2, double lat2) {
  double d_lat = M_PI * (lat2 - lat1) / 180.0;
  double d_lon = M_PI * (lon2 - lon1) / 180.0;
  double a = std::sin(d_lat / 2) * std::sin(d_lat / 2) + std::cos(M_PI*(lat1)/180.0) * std::cos(M_PI*(lat2)/180.0) * std::sin(d_lon / 2) * std::sin(d_lon / 2);
  return 2 * kRad * std::asin(std::sqrt(a));
}

inline double KmToUnit(double km, const std::string& unit) {
  if (unit == "m") {
    return km * 1000.0;
  }
  if (unit == "mi") {
    return km / 1.609344;
  }
  if (unit == "ft") {
    return km * 3280.839895;
  }
  return km;
}

inline double UnitToKm(double val, const std::string& unit) {
  if (unit == "m") {
    return val / 1000.0;
  }
  if (unit == "mi") {
    return val * 1.609344;
  }
  if (unit == "ft") {
    return val / 3280.839895;
  }
  return val;
}

inline bool ValidUnit(const std::string& u) {
  return u == "m" || u == "km" || u == "mi" || u == "ft";
}