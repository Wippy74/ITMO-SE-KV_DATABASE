#include "../include/kernel.h"

void DataBase::SetEntry(const std::string& key, Entry entry) {
  storage_[key] = std::move(entry);
}

bool DataBase::DelEntry(const std::string& key) {
  return storage_.erase(key) > 0;
}

bool DataBase::IsExpired(const Entry& entry) const {
  if (!entry.expires_at) {
    return false;
  }
  return std::chrono::steady_clock::now() >= *entry.expires_at;
}

Entry* DataBase::GetEntry(const std::string& key) {
  auto it = storage_.find(key);
  if (it == storage_.end()) {
    return nullptr;
  }
  if (IsExpired(it->second)) {
    storage_.erase(it);
    return nullptr;
  }
  return &it->second;
}

std::vector<std::string> DataBase::Keys() const {
  std::vector<std::string> res;
  res.reserve(storage_.size());
  for (const auto& [key, _] : storage_) {
    res.push_back(key);
  }
  return res;
}

std::size_t DataBase::Size() const {
  return storage_.size();
}
