#include "../include/kernel.h"
#include <algorithm>
#include <iostream>

size_t DataBase::CountEntryMemory(const std::string& key,const Entry& entry) {
  size_t size = key.capacity() + sizeof(Entry) + 64;
  std::visit(overloaded{
    [&](const std::string& s) {
      size += s.capacity();
    },
    [&](const std::deque<std::string>& l) {
      size += sizeof(l);
      for (const auto& elem : l) {
        size += elem.capacity() + sizeof(std::string);
      }
    },
    [&](const std::unordered_set<std::string>& st) {
      size += sizeof(st);
      for (const auto& elem : st) {
        size += elem.capacity() + sizeof(std::string) + 32;
      }
    },
    [&](const std::unordered_map<std::string, GeoType>& geo) {
      size += sizeof(geo);
      for (const auto& [name, _] : geo) {
        size += name.capacity() + sizeof(GeoType) + 32;
      }
    },
  }, entry.value);
  return size;
}

bool DataBase::SetEntry(const std::string& key, Entry entry) {
  if (memory_.GetMax() == 0) {
    storage_[key] = std::move(entry);
    return true;
  }
  auto it = storage_.find(key);
  if (it != storage_.end()) {
    memory_.Sub(it->second.mem_);
  }
  size_t newSize = CountEntryMemory(key, entry);
  if (!memory_.CanAllocate(newSize)) {
    if (it != storage_.end()) {
      memory_.Add(it->second.mem_);
    }
    std::cerr << "(error) OOM command not allowed when used memory > 'maxmemory'\n";
    return false;
  }
  entry.mem_ = newSize;
  memory_.Add(newSize);
  storage_[key] = std::move(entry);
  return true;
}

bool DataBase::DelEntry(const std::string& key) {
  auto it = storage_.find(key);
  if (it == storage_.end()) {
    return false;
  }
  memory_.Sub(it->second.mem_);
  storage_.erase(it);
  return true;
}

Entry* DataBase::GetEntry(const std::string& key) {
  auto it = storage_.find(key);
  if (it == storage_.end()) {
    return nullptr;
  }
  if (IsExpired(it->second)) {
    memory_.Sub(it->second.mem_);
    storage_.erase(it);
    return nullptr;
  }
  return &it->second;
}

void DataBase::RecountEntry(const std::string& key) {
  if (memory_.GetMax() == 0) {
    return;
  }
  auto it = storage_.find(key);
  if (it == storage_.end()) {
    return;
  }
  size_t oldMem = it->second.mem_;
  size_t newMem = CountEntryMemory(key, it->second);
  it->second.mem_ = newMem;
  if (newMem > oldMem) {
    memory_.Add(newMem - oldMem);
  } else {
    memory_.Sub(oldMem - newMem);
  }
}

bool DataBase::RequireMemory(size_t extra) const {
  if (memory_.CanAllocate(extra)) {
    return true;
  }
  std::cerr << "(error) OOM command not allowed when used memory > 'maxmemory'\n";
  return false;
}

std::vector<std::string> DataBase::Keys() const {
  std::vector<std::string> res;
  res.reserve(storage_.size());
  for (const auto& [key, _] : storage_) {
    res.push_back(key);
  }
  return res;
}

std::vector<std::string> DataBase::LiveKeys() const {
  std::vector<std::string> res;
  res.reserve(storage_.size());
  for (const auto& [key, entry] : storage_) {
    if (!IsExpired(entry)) {
      res.push_back(key);
    }
  }
  return res;
}

size_t DataBase::Size() const {
  return storage_.size();
}

void DataBase::FlushDB() {
  storage_.clear();
  memory_.Sub(memory_.GetUsed());
}

bool DataBase::IsExpired(const Entry& entry) const {
  if (!entry.expires_at) {
    return false;
  }
  return std::chrono::steady_clock::now() >= *entry.expires_at;
}