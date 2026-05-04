#include "../include/parser.h"

#include <cctype>
#include <sstream>
#include <string>
#include <iostream>

namespace {

std::string ToUpper(std::string s) {
  for (char& c : s) {
    c = static_cast<char>(std::toupper(c));
  }
  return s;
}

bool IsDelim(char c) {
  return c == ' ' || c == '\t' || c == '\r';
}

void EscSequence(const std::string& line, std::size_t& pos, std::string& out) {
  if (pos >= line.size()) {
    std::cerr << "Incorrect end of string after backslash" << '\n';
  }
  const char esc = line[pos++];
  switch (esc) {
    case '"': out += '"'; return;
    case '\\': out += '\\'; return;
    case 'n': out += '\n'; return;
    case 'r': out += '\r'; return;
    case 't': out += '\t'; return;
    default:
      out += esc;
      return;
  }
}

void ReadEnqouted(const std::string& line, std::size_t& pos, std::string& out) {
  while (pos < line.size()) {
    const char ch = line[pos];
    if (ch == '"') {
      ++pos; 
      return;
    }
    if (ch == '\\') {
      ++pos;
      EscSequence(line, pos, out);
    } else {
      out += ch;
      ++pos;
    }
  }
  std::cerr << "Unclosed quote" << '\n';
}

bool ReadTok(const std::string& line, std::size_t& pos, std::string& out) {
  while (pos < line.size() && IsDelim(line[pos])) {
    ++pos;
  }
  if (pos >= line.size()) {
    return false;
  }
  out.clear();
  while (pos < line.size() && !IsDelim(line[pos])) {
    if (line[pos] == '"') {
      ++pos;
      ReadEnqouted(line, pos, out);
    } else {
      while (line[pos] != '"') {
        out += line[pos++];
      }
    }
  }
  return true;
}

} // anonymous namespace


Commands ParseCommand(const std::string& line) {
  Commands result;
  std::size_t pos = 0;
  std::string token;

  if (!ReadTok(line, pos, token)) {
    return result;
  }
  result.name = ToUpper(token);

  while (ReadTok(line, pos, token)) {
    result.args.push_back(std::move(token));
  }
  return result;
}