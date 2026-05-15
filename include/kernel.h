#pragma once
#include "memory_disp.h"
#include "types/types.h"
#include <string>
#include <vector>
#include <unordered_map>

class DataBase {
public:
  bool SetEntry(const std::string& key, Entry entry);
  bool DelEntry(const std::string& key);
  bool IsExpired(const Entry& entry) const;
  
  Entry* GetEntry(const std::string& key);
  std::vector<std::string> Keys() const;
  std::size_t Size() const;
  void FlushDB();

  static std::size_t CountEntryMemory(const std::string& key, const Entry& entry);
  void RecountEntry(const std::string& key);
  bool RequireMemory(std::size_t extra) const;

  MemoryDispatcher& Memory() {
    return memory_;
  }

  const MemoryDispatcher& Memory() const {
    return memory_;
  }
private:
  std::unordered_map<std::string, Entry> storage_;
  MemoryDispatcher memory_;
};