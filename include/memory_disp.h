#pragma once
#include <cstddef>
#include <algorithm>

class MemoryDispatcher {
public:
  void SetMax(size_t max) {
    max_ = max;
  }
  size_t GetMax() const {
    return max_;
  }
  size_t GetUsed() const {
    return used_;
  }

  void Add(size_t bytes) {
    used_ += bytes;
  }
  void Sub(size_t bytes) {
    used_ -= std::min(used_, bytes);
  }
  bool CanAllocate(size_t bytes) const {
    if (max_ == 0) {
      return true;
    }
    return used_ + bytes <= max_;
  }
private:
  size_t used_ = 0;
  size_t max_;
};