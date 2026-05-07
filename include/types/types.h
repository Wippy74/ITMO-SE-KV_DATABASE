#pragma once
#include <string>
#include <unordered_set>
#include <deque>
#include <unordered_map>
#include <variant>
#include <optional>
#include <chrono>

struct GeoType {
  double longitude;
  double latitude;
};

struct Entry {
  std::variant<std::string, std::deque<std::string>, std::unordered_set<std::string>, std::unordered_map<std::string, GeoType>> value;
  std::optional<std::chrono::steady_clock::time_point> expires_at;
  std::size_t mem_ = 0;
};