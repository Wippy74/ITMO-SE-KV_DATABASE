#include "../include/parser.h"
#include <cctype>

Command ParseCommand(const std::string& line) {
  size_t pos = 0;
  const size_t n = line.size();
  auto skip = [&] {
    while (pos < n && (line[pos] == ' ' || line[pos] == '\t')) {
      ++pos;
    }
  };
  Command result;
  while (true) {
    skip();
    if (pos >= n) {
      break;
    }
    std::string token;
    if (line[pos] == '"') {
      ++pos;
      while (pos < n && line[pos] != '"') {
        token += line[pos++];
      }
      if (pos < n) {
        ++pos;
      }
    } else {
      while (pos < n && line[pos] != ' ' && line[pos] != '\t') {
        token += line[pos++];
      }
    }
    if (result.name.empty()) {
      for (char& c : token) {
        c = static_cast<char>(std::toupper(c));
      }
      result.name = std::move(token);
    } else {
      result.args.push_back(std::move(token));
    }
  }
  return result;
}
