// Copy-on-Write mechanism probe.
// Two holders share a page; one writes; the other still sees the original.

#include "../model/minimal/page.hpp"
#include <cstdio>

int main() {
  auto page = std::make_shared<Page>("original");
  page->refcount = 0;   // CoWPage ctor will increment

  CoWPage a(page);
  CoWPage b = a;        // share

  int shared_refcount = a.refcount();
  std::string before_a = a.read();
  std::string before_b = b.read();

  // a writes; should clone
  a.write("modified-by-a");

  bool isolated = (a.read() == "modified-by-a") && (b.read() == "original");
  bool a_exclusive = (a.refcount() == 1);
  bool b_still_shared_or_exclusive = (b.refcount() >= 1);

  std::printf("{\n");
  std::printf("  \"probe\": \"cow\",\n");
  std::printf("  \"shared_refcount_before_write\": %d,\n", shared_refcount);
  std::printf("  \"a_after\": \"%s\",\n", a.read().c_str());
  std::printf("  \"b_after\": \"%s\",\n", b.read().c_str());
  std::printf("  \"isolated\": %s,\n", isolated ? "true" : "false");
  std::printf("  \"a_exclusive\": %s,\n", a_exclusive ? "true" : "false");
  std::printf("  \"note\": \"toy CoW; write clones if refcount > 1\"\n");
  std::printf("}\n");

  return isolated && a_exclusive ? 0 : 1;
}
