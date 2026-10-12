// Minimal LSM model — in-memory toy for learning.

#pragma once
#include <map>
#include <string>
#include <vector>
#include <optional>

using Key = std::string;
using Value = std::string;

class MemTable {
public:
  void put(const Key& k, const Value& v) { data_[k] = v; }
  std::optional<Value> get(const Key& k) const {
    auto it = data_.find(k);
    return it == data_.end() ? std::nullopt : std::optional<Value>(it->second);
  }
  bool empty() const { return data_.empty(); }
  size_t size() const { return data_.size(); }
  const std::map<Key, Value>& data() const { return data_; }
  void clear() { data_.clear(); }

private:
  std::map<Key, Value> data_;
};

struct SSTable {
  int id = 0;
  int level = 0;
  std::map<Key, Value> data;
  Key min_key, max_key;
};

class ToyLSM {
public:
  explicit ToyLSM(size_t mem_limit = 4) : mem_limit_(mem_limit), next_id_(1) {}

  void put(const Key& k, const Value& v) {
    mem_.put(k, v);
    if (mem_.size() >= mem_limit_) flush();
  }

  std::optional<Value> get(const Key& k) const {
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
    mem_.clear();
  }

  size_t l0_count() const { return l0_.size(); }
  size_t mem_size() const { return mem_.size(); }

private:
  MemTable mem_;
  std::vector<SSTable> l0_;
  size_t mem_limit_;
  int next_id_;
};

inline ToyLSM* make_toy_lsm(size_t mem_limit = 4) { return new ToyLSM(mem_limit); }
