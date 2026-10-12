// Minimal LSM model — interfaces for learning architecture.
// In-memory only; SSTable is simulated.

#pragma once
#include <map>
#include <string>
#include <vector>
#include <optional>

using Key = std::string;
using Value = std::string;

// Ordered in-memory table
class MemTable {
public:
  void put(const Key& k, const Value& v);
  std::optional<Value> get(const Key& k) const;
  bool empty() const { return data_.empty(); }
  size_t size() const { return data_.size(); }
  // For flush
  const std::map<Key, Value>& data() const { return data_; }

private:
  std::map<Key, Value> data_;
};

// Immutable sorted run (simulated SSTable)
struct SSTable {
  int id = 0;
  int level = 0;
  std::map<Key, Value> data;   // already sorted
  Key min_key, max_key;
};

// Compaction policy
class CompactionPolicy {
public:
  virtual ~CompactionPolicy() = default;
  virtual bool should_compact(const std::vector<SSTable>& level) const = 0;
};

class SizeBasedPolicy : public CompactionPolicy {
public:
  explicit SizeBasedPolicy(size_t threshold) : threshold_(threshold) {}
  bool should_compact(const std::vector<SSTable>& level) const override;
private:
  size_t threshold_;
};
