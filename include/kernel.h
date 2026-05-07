#include "memory_disp.h"
#include "./types/types.h"
#include <vector>

class DataBase {
public:
  void SetEntry(const std::string& key, Entry entry);
  bool DelEntry(const std::string& key);
  bool IsExpired(const Entry& entry) const;
  bool CheckMemory(std::size_t diff) const;
  
  Entry* GetEntry(const std::string& key);
  std::vector<std::string> Keys() const;
  std::size_t Size() const;
private:
  std::unordered_map<std::string, Entry> storage_;
  MemoryDispatcher memory_;
};