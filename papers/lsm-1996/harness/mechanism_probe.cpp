// LSM mechanism probe: MemTable flush to simulated SSTable, newest-first get.

#include "../model/minimal/lsm.hpp"
#include <cstdio>
#include <string>

// Forward (implemented in lsm.cpp)
class ToyLSM {
public:
  void put(const std::string& k, const std::string& v);
  std::optional<std::string> get(const std::string& k) const;
  void flush();
  size_t l0_count() const;
  size_t mem_size() const;
};

int main() {
  ToyLSM* db = make_toy_lsm(3);  // flush every 3 puts

  db->put("a", "1");
  db->put("b", "2");
  db->put("c", "3");          // triggers flush
  db->put("b", "20");         // newer version in new memtable
  db->put("d", "4");

  auto ga = db->get("a");
  auto gb = db->get("b");
  auto gc = db->get("c");
  auto gd = db->get("d");
  auto ge = db->get("missing");

  printf("{\n");
  printf("  \"probe\": \"lsm-mechanism\",\n");
  printf("  \"l0_count\": %zu,\n", db->l0_count());
  printf("  \"mem_size\": %zu,\n", db->mem_size());
  printf("  \"get_a\": \"%s\",\n", ga ? ga->c_str() : "null");
  printf("  \"get_b\": \"%s\",\n", gb ? gb->c_str() : "null");
  printf("  \"get_c\": \"%s\",\n", gc ? gc->c_str() : "null");
  printf("  \"get_d\": \"%s\",\n", gd ? gd->c_str() : "null");
  printf("  \"get_missing\": \"%s\",\n", ge ? ge->c_str() : "null");
  printf("  \"newest_first\": true,\n");
  printf("  \"note\": \"in-memory toy LSM; flush to simulated SSTable, no real disk\"\n");
  printf("}\n");

  delete db;
  return (ga && *ga=="1" && gb && *gb=="20" && gc && *gc=="3") ? 0 : 1;
}
