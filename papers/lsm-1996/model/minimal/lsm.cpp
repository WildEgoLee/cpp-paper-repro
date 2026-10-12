// Minimal in-memory LSM for learning.
// MemTable → flush to simulated SSTable. No real disk.

#include "lsm.hpp"
#include <algorithm>

void MemTable::put(const Key& k, const Value& v) {
  data_[k] = v;
}

std::optional<Value> MemTable::get(const Key& k) const {
  auto it = data_.find(k);
  if (it == data_.end()) return std::nullopt;
  return it->second;
}

bool SizeBasedPolicy::should_compact(const std::vector<SSTable>& level) const {
  return level.size() >= threshold_;
}

// Simple DB that owns memtable + L0 sstables
class ToyLSM {
public:
  explicit ToyLSM(size_t mem_limit = 4)
    : mem_limit_(mem_limit), next_id_(1) {}

  void put(const Key& k, const Value& v) {
    mem_.put(k, v);
    if (mem_.size() >= mem_limit_) flush();
  }

  std::optional<Value> get(const Key& k) const {
    // Newest first: memtable, then L0 (newest to oldest)
    if (auto v = mem_.get(k)) return v;
    for (auto it = l0_.rbegin(); it != l0_.rend(); ++it) {
      auto jt = it->data.find(k);
      if (jt != it->data.end()) return jt->second;
    }
    return std::nullopt;
  }

  void flush() {
    if (mem_.empty()) return;
    SSTable sst;
    sst.id = next_id_++;
    sst.level = 0;
    sst.data = mem_.data();
    if (!sst.data.empty()) {
      sst.min_key = sst.data.begin()->first;
      sst.max_key = sst.data.rbegin()->first;
    }
    l0_.push_back(std::move(sst));
    mem_ = MemTable{};  // reset
  }

  size_t l0_count() const { return l0_.size(); }
  size_t mem_size() const { return mem_.size(); }

private:
  MemTable mem_;
  std::vector<SSTable> l0_;
  size_t mem_limit_;
  int next_id_;
};

// Exposed for the probe
ToyLSM* make_toy_lsm(size_t mem_limit) { return new ToyLSM(mem_limit); }
