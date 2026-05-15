#pragma once
#include <variant>
#include <string>
#include <vector>
#include <optional>

struct NilResult {};
struct ExitSignal {};
struct SimpleString {
  std::string value;
};
struct ResString {
  std::string value;
}; 

using ReturnResult = std::variant<ExitSignal, NilResult, size_t, SimpleString, ResString, std::vector<std::string>>;

using OptionalResult = std::optional<ReturnResult>;

inline OptionalResult MakeNil() {
  return NilResult{};
}
inline OptionalResult MakeOk() {
  return SimpleString{"OK"};
}
inline OptionalResult MakeSize(size_t n) {
  return n;
}
inline OptionalResult MakeString(const std::string& s) {
  return ResString{s};
}
inline OptionalResult MakeStatus(const std::string& s) {
  return SimpleString{s};
}
inline OptionalResult MakeArray(std::vector<std::string> v) {
  return v;
}